S=/private/tmp/claude-501/-Users-justinzaun-Development-logic/b09aff1a-5b91-4531-aa82-d3e9dc3331bb/scratchpad
objs=""; for t in $(ls $S/bcov | grep -E '_(test|check)$'); do objs="$objs -object $S/bcov/$t"; done
xcrun llvm-cov show $objs -instr-profile $S/bcov/all.profdata -sources "$@"
