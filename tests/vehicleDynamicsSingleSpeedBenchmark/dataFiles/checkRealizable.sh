set -e
../../../libs/slugs_unstable/tools/StructuredSlugsParser/compiler.py slugsExample.structuredslugs > slugsExample.slugsin
../../../libs/slugs_unstable/src/slugs --nonDeterministicMotion slugsExample.slugsin slugsExample.bdd
