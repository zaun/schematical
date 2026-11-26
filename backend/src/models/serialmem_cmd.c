/* models/serialmem_cmd.c: part of the model in models/serialmem.c (split by plan-68-I/J). */
#include "serialmem_int.h"
/* ---- Commands ------------------------------------------------------------------------------ */

static Cmd lookup(const SmState *s, uint8_t op) {
    const SerialmemDevice *d = s->dev;

    if (d->type == SM_FLASH) {
        switch (op) {
            case 0x03: return C_READ;
            case 0x0B: return C_FREAD;
            case 0x02: return C_WRITE;
            case 0x81: return d->t_page_erase > 0 ? C_ERASE_PAGE : C_IGNORE;
            case 0x20: return C_ERASE_4K;
            case 0x52: return C_ERASE_32K;
            case 0xD8: return d->t_block64 > 0 ? C_ERASE_64K : C_ERASE_32K;
            case 0x60:
            case 0xC7: return C_ERASE_CHIP;
            case 0x06: return C_WREN;
            case 0x04: return C_WRDI;
            case 0x05: return C_RDSR;
            case 0x01: return C_WRSR;
            case 0x31: return C_WRSR2;
            case 0x9F: return C_RDID;
            case 0xB9: return C_DPD;
            case 0xAB: return C_RESUME;
            case 0xF0: return C_RESET;
            case 0x36: return d->prot == PROT_SECTOR ? C_PROTECT : C_IGNORE;
            case 0x39: return d->prot == PROT_SECTOR ? C_UNPROTECT : C_IGNORE;
            case 0x3C: return d->prot == PROT_SECTOR ? C_READ_SPR : C_IGNORE;
            default: return C_IGNORE;
        }
    }
    switch (op) {
        case 0x06: return C_WREN;
        case 0x04: return C_WRDI;
        case 0x05: return C_RDSR;
        case 0x01: return C_WRSR;
        case 0x03: return C_READ;
        case 0x02: return C_WRITE;
        case 0x9F: return d->jedec_len > 0 ? C_RDID : C_IGNORE;
        case 0x0B: return d->fast_read ? C_FREAD : C_IGNORE;
        case 0xB9: return d->sleep ? C_SLEEP : C_IGNORE;
        default: return C_IGNORE;
    }
}

static void decode(SmState *s, uint8_t op, uint8_t known) {
    const SerialmemDevice *d = s->dev;
    uint8_t need = d->a8 ? 0xF7 : 0xFF;
    Tx *t = &s->tx;
    Cmd c;

    t->op = op;
    t->op_known = known;
    if (s->gate_ignore) {
        t->cmd = C_IGNORE;
        return;
    }
    if ((known & need) != need) {
        t->cmd = C_UNKNOWN;
        return;
    }
    c = lookup(s, (uint8_t)(op & need));
    if (s->busy && c != C_RDSR) {
        c = C_IGNORE;
    }
    if (s->dpd && c != C_RESUME) {
        c = C_IGNORE;
    }
    t->cmd = c;
    switch (c) {
        case C_READ:
        case C_WRITE:
            t->naddr = d->address_bytes;
            break;
        case C_FREAD:
            t->naddr = d->address_bytes;
            t->dummy = 1;
            break;
        case C_ERASE_PAGE:
        case C_ERASE_4K:
        case C_ERASE_32K:
        case C_ERASE_64K:
        case C_PROTECT:
        case C_UNPROTECT:
        case C_READ_SPR:
            t->naddr = 3;
            break;
        default:
            break;
    }
}

static void address_done(SmState *s) {
    Tx *t = &s->tx;

    if (s->dev->a8) {
        t->addr |= (uint32_t)((t->op >> 3) & 1u) << 8;
        if ((t->op_known >> 3) & 1u) {
            t->addr_known |= (uint32_t)1 << 8;
        }
    }
    t->addr &= s->mask;
    t->addr_known &= s->mask;
    t->addr_done = 1;
    t->unknown_addr = t->addr_known != s->mask;
}

/* One byte of a WRITE or PROGRAM data phase. */
static void data_byte(SmState *s, uint8_t data, uint8_t known) {
    Tx *t = &s->tx;
    uint32_t page = s->dev->page_size;

    if (t->data_n == 0) {
        t->first = data;
        t->first_known = known;
    }
    if (s->dev->type == SM_FRAM) {
        int perm;

        if (t->stopped) {
            return;
        }
        if (t->unknown_addr) {
            if (s->wel != T0) {
                smem_corrupt_matching(s, t->addr, t->addr_known, 1);
            }
            t->stopped = 1;
            return;
        }
        perm = smem_t_and(s->wel, smem_t_not(smem_range_protected(s, t->addr, 1)));
        if (perm == T0) {
            t->stopped = 1;
            return;
        }
        if (perm == TX || s->spi.x) {
            known = 0;
        }
        smem_put_byte(s, t->addr, data, known, 0);
        t->addr = (t->addr + 1) & s->mask;
        t->data_n++;
        return;
    }
    if (t->data_n == 0) {
        s->page_base = t->addr & ~(page - 1);
        s->page_offset = t->addr & (page - 1);
        memset(s->buf_loaded, 0, page);
    }
    {
        uint32_t index = (s->page_offset + t->data_n) & (page - 1);

        s->buf_value[index] = data;
        s->buf_known[index] = known;
        s->buf_loaded[index] = 1;
    }
    t->data_n++;
}

void smem_sm_received(void *context, unsigned index, uint8_t data, uint8_t known) {
    SmState *s = context;
    Tx *t = &s->tx;

    if (index == 0) {
        decode(s, data, known);
        if (t->naddr == 0) {
            t->addr_done = 1;
        }
        return;
    }
    if (t->cmd == C_NONE || t->cmd == C_IGNORE || t->cmd == C_UNKNOWN) {
        return;
    }
    if (t->addr_n < t->naddr) {
        t->addr = (t->addr << 8) | data;
        t->addr_known = (t->addr_known << 8) | known;
        if (++t->addr_n == t->naddr) {
            address_done(s);
        }
        return;
    }
    if (t->dummy > 0 && index <= t->naddr + t->dummy) {
        return;
    }
    switch (t->cmd) {
        case C_WRITE:
            if (t->addr_done) {
                data_byte(s, data, known);
            }
            break;
        case C_WRSR:
        case C_WRSR2:
        case C_RESET:
            if (t->data_n == 0) {
                t->first = data;
                t->first_known = known;
            }
            t->data_n++;
            break;
        default:
            break;
    }
}

SerialSpiByte smem_sm_next(void *context, unsigned index) {
    SmState *s = context;
    Tx *t = &s->tx;
    SerialSpiByte out = {0, 0, 0};
    uint8_t v, k;

    if (index == 0) {
        return out;
    }
    if (t->cmd == C_UNKNOWN) {
        out.drive = 0xFF;
        return out;
    }
    switch (t->cmd) {
        case C_RDSR:
            smem_status_byte(s, s->dev->type == SM_FLASH ? (index - 1) % 2 : 0, &v, &k);
            out.data = v;
            out.known = k;
            out.drive = 0xFF;
            break;
        case C_READ:
        case C_FREAD:
            if (index > t->naddr + t->dummy) {
                out.drive = 0xFF;
                if (t->unknown_addr || !t->addr_done) {
                    break;
                }
                out.data = s->value[t->addr];
                out.known = s->known[t->addr];
                t->addr = (t->addr + 1) & s->mask;
            }
            break;
        case C_RDID:
            if (index - 1 < s->dev->jedec_len) {
                out.data = s->dev->jedec[index - 1];
                out.known = 0xFF;
                out.drive = 0xFF;
            }
            break;
        case C_READ_SPR:
            if (index > t->naddr && t->addr_done) {
                int p = t->unknown_addr ? TX : s->sprot[(t->addr / s->dev->protect_size) % s->sprot_count];

                out.drive = 0xFF;
                out.data = p == T1 ? 0xFF : 0x00;
                out.known = p == TX ? 0x00 : 0xFF;
            }
            break;
        default:
            break;
    }
    return out;
}

void smem_sm_begin(void *context) {
    SmState *s = context;

    memset(&s->tx, 0, sizeof s->tx);
    s->tx.cmd = C_NONE;
    s->wp_fell = 0;
    s->gate_ignore = 0;
    if (s->asleep) {
        s->asleep = 0;
        s->wake_until = smem_now_of(s) + s->dev->t_wake;
        s->gate_ignore = 1;
    } else if (smem_now_of(s) < s->wake_until) {
        s->gate_ignore = 1;
    }
}

/* ---- End of a transaction ------------------------------------------------------------------- */

void smem_set_all_sectors(SmState *s, int value) {
    uint32_t i;

    for (i = 0; i < s->sprot_count; i++) {
        s->sprot[i] = (uint8_t)value;
    }
}

static void end_eeprom(SmState *s, unsigned bits) {
    const SerialmemDevice *d = s->dev;
    Tx *t = &s->tx;
    int x = s->spi.x;
    int wp = smem_control(s, SM_WP);
    int bytes_ok = bits % 8 == 0 && bits >= 8;

    if (t->cmd == C_UNKNOWN) {
        s->wel = TX;
        return;
    }
    if (x && (t->cmd == C_WREN || t->cmd == C_WRDI || t->cmd == C_WRSR || t->cmd == C_SLEEP)) {
        s->wel = TX;   /* what an X transaction did is unknown */
        return;
    }
    switch (t->cmd) {
        case C_WREN:
            if (!bytes_ok) {
                break;
            }
            if (d->prot == PROT_WP && wp != T1) {
                if (wp == TX) {
                    s->wel = TX;
                }
                break;
            }
            s->wel = T1;
            break;
        case C_WRDI:
            if (bytes_ok) {
                s->wel = T0;
            }
            break;
        case C_SLEEP:
            if (bytes_ok) {
                s->asleep = 1;
            }
            break;
        case C_WRSR: {
            int writable;
            int perm;

            if (!bytes_ok || t->data_n < 1) {
                break;
            }
            if (s->wp_fell) {
                wp = T0;
            }
            writable = d->prot == PROT_WP ? wp : smem_t_or(smem_t_not(s->lock), wp);
            perm = smem_t_and(s->wel, writable);
            if (perm == T0) {
                break;
            }
            s->pend.kind = P_WRSR;
            s->pend.value = t->first;
            s->pend.value_known = t->first_known;
            s->pend.maybe = perm == TX;
            if (d->type == SM_FRAM) {
                smem_apply_pending(s);
                s->wel = T0;
            } else {
                smem_start_op(s, d->t_write);
            }
            break;
        }
        case C_WRITE: {
            int perm;
            uint32_t span;

            if (!t->addr_done) {
                break;
            }
            if (d->type == SM_FRAM) {
                if (s->wel != T0) {
                    s->wel = T0;
                }
                break;
            }
            if (!bytes_ok || t->data_n < 1) {
                break;
            }
            if (s->wp_fell) {
                wp = T0;
            }
            perm = smem_t_and(s->wel, smem_t_not(t->unknown_addr ? T0 : smem_range_protected(s, t->addr, 1)));
            if (d->prot == PROT_WP) {
                perm = smem_t_and(perm, wp);
            }
            if (perm == T0) {
                break;
            }
            span = d->page_size;
            if (t->unknown_addr) {
                s->pend.kind = P_CORRUPT;
                s->pend.addr = t->addr;
                s->pend.known = t->addr_known;
                s->pend.length = span;
            } else {
                s->pend.kind = P_PAGE;
                s->pend.maybe = perm == TX || x;
            }
            smem_start_op(s, d->t_write);
            break;
        }
        default:
            break;
    }
}

/* A flash command that stops WEL: the opcode was complete. */
static void end_flash(SmState *s, unsigned bits) {
    const SerialmemDevice *d = s->dev;
    Tx *t = &s->tx;
    int x = s->spi.x;
    int bytes_ok = bits % 8 == 0 && bits >= 8;
    int wel = s->wel;
    int wp = smem_control(s, SM_WP);
    uint32_t length = 0;
    int perm;
    int prot;

    if (t->cmd == C_UNKNOWN) {
        s->wel = TX;
        return;
    }
    switch (t->cmd) {
        case C_WREN:
            if (bytes_ok) {
                s->wel = x ? TX : T1;
            }
            return;
        case C_WRDI:
            if (bytes_ok) {
                s->wel = x ? TX : T0;
            }
            return;
        case C_DPD:
            if (bytes_ok) {
                s->dpd = 1;
            }
            return;
        case C_RESUME:
            if (bytes_ok) {
                s->dpd = 0;
            }
            return;
        case C_RESET:
            if (bits == 16 && t->data_n == 1 && t->first_known == 0xFF && t->first == 0xD0 &&
                s->rste && !x) {
                s->wel = T0;
                s->lock = T0;
                s->dpd = 0;
                if (d->prot == PROT_SECTOR) {
                    smem_set_all_sectors(s, T1);
                }
            }
            return;
        case C_WRSR2:
            if (wel == T0) {
                return;
            }
            s->wel = T0;
            /* An unknown RSTE write leaves RSTE as it was. */
            if (bytes_ok && t->data_n >= 1 && !x && wel == T1 && (t->first_known & 0x10)) {
                s->rste = (t->first >> 4) & 1u;
            }
            return;
        case C_WRSR: {
            if (wel == T0) {
                return;
            }
            s->wel = T0;
            if (!bytes_ok || t->data_n < 1) {
                return;
            }
            if (d->prot == PROT_SECTOR) {
                uint8_t need = 0xBC;   /* bit 7 and bits 5:2 */

                if (x || wel == TX || wp == TX || s->lock == TX || (t->first_known & need) != need) {
                    s->lock = TX;
                    smem_set_all_sectors(s, TX);
                    return;
                }
                if (wp == T0 && s->lock == T1) {
                    return;   /* hard locked */
                }
                if (s->lock == T0) {
                    unsigned g = (t->first >> 2) & 0xFu;

                    if (g == 0) {
                        smem_set_all_sectors(s, T0);
                    } else if (g == 0xF) {
                        smem_set_all_sectors(s, T1);
                    }
                }
                s->lock = (t->first & 0x80) ? T1 : T0;
                return;
            }
            if (x || wel == TX || wp == TX || s->lock == TX) {
                s->pend.kind = P_WRSR_FLASH;
                s->pend.maybe = 1;
                smem_start_op(s, d->t_wrsr);
                return;
            }
            if (wp == T0 && s->lock == T1) {
                return;
            }
            s->pend.kind = P_WRSR_FLASH;
            s->pend.value = t->first;
            s->pend.value_known = t->first_known;
            s->pend.maybe = 0;
            smem_start_op(s, d->t_wrsr);
            return;
        }
        case C_PROTECT:
        case C_UNPROTECT:
            if (wel == T0) {
                return;
            }
            s->wel = T0;
            if (!bytes_ok || !t->addr_done) {
                return;
            }
            perm = smem_t_and(wel, smem_t_not(s->lock));
            if (perm == T0) {
                return;
            }
            if (t->unknown_addr || perm == TX || x) {
                smem_set_all_sectors(s, TX);
                return;
            }
            s->sprot[(t->addr / d->protect_size) % s->sprot_count] =
                t->cmd == C_PROTECT ? T1 : T0;
            return;
        case C_WRITE:
        case C_ERASE_PAGE:
        case C_ERASE_4K:
        case C_ERASE_32K:
        case C_ERASE_64K:
        case C_ERASE_CHIP:
            break;
        default:
            return;
    }
    /* Program and erase. */
    if (wel == T0) {
        return;
    }
    s->wel = T0;
    if (!bytes_ok) {
        return;
    }
    {
        SimTime duration = 0;

        switch (t->cmd) {
            case C_WRITE:
                if (!t->addr_done || t->data_n < 1) {
                    return;
                }
                length = d->page_size;
                duration = t->data_n == 1 ? d->t_byte_prog : d->t_page_prog;
                break;
            case C_ERASE_PAGE:
                length = d->page_size;
                duration = d->t_page_erase;
                break;
            case C_ERASE_4K:
                length = 4096;
                duration = d->t_sector;
                break;
            case C_ERASE_32K:
                length = 32768;
                duration = d->t_block32;
                break;
            case C_ERASE_64K:
                length = 65536;
                duration = d->t_block64;
                break;
            default:
                length = d->size;
                duration = d->t_chip;
                break;
        }
        if (t->cmd != C_WRITE && t->cmd != C_ERASE_CHIP && !t->addr_done) {
            return;
        }
        if (length > d->size) {
            length = d->size;
        }
        if (t->cmd == C_ERASE_CHIP) {
            prot = smem_range_protected(s, 0, d->size);
        } else if (t->unknown_addr) {
            prot = T0;
        } else {
            uint32_t start = t->addr & ~(length - 1);

            prot = t->cmd == C_WRITE ? smem_range_protected(s, t->addr, 1) : smem_range_protected(s, start, length);
        }
        perm = smem_t_and(wel, smem_t_not(prot));
        if (perm == T0) {
            return;
        }
        if (t->unknown_addr) {
            s->pend.kind = P_CORRUPT;
            s->pend.addr = t->addr;
            s->pend.known = t->addr_known;
            s->pend.length = length;
        } else if (t->cmd == C_WRITE) {
            s->pend.kind = P_PAGE;
            s->pend.maybe = perm == TX || x;
        } else {
            s->pend.kind = P_ERASE;
            s->pend.addr = t->cmd == C_ERASE_CHIP ? 0 : (t->addr & ~(length - 1));
            s->pend.length = length;
            s->pend.maybe = perm == TX || x;
        }
        smem_start_op(s, duration);
    }
}

void smem_sm_end(void *context, unsigned bits) {
    SmState *s = context;
    Tx *t = &s->tx;

    if (t->cmd == C_IGNORE || t->cmd == C_NONE) {
        return;
    }
    if (s->dev->type == SM_FLASH) {
        end_flash(s, bits);
    } else {
        end_eeprom(s, bits);
    }
}

