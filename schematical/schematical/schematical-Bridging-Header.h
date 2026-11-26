//
//  schematical-Bridging-Header.h
//  schematical
//
//  Exposes the backend (../backend/src, compiled into the app) to Swift:
//  the part library, the circuit simulator, the voltage check, the ERC pin types and the
//  load-file parser.
//

#ifndef SCHEMATICAL_BRIDGING_HEADER_H
#define SCHEMATICAL_BRIDGING_HEADER_H

#include "parts.h"
#include "circuit.h"
#include "voltage.h"
#include "erc.h"
#include "memory_image.h"

#endif
