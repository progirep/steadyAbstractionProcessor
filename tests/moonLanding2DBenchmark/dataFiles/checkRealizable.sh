set -e
../../../libs/slugs/tools/StructuredSlugsParser/compiler.py slugsExample.structuredslugs > slugsExample.slugsin
../../../libs/slugs/src/slugs --nonDeterministicMotion slugsExample.slugsin slugsExample.bdd
