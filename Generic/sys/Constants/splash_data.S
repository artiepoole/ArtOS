.section .rawdata
.global SPLASH_DATA
SPLASH_DATA:
# This assumes the gcc call is from within one deep dir such as with when using cmake. Change as necessary
.incbin "../external_resources/inkscape/splash_logo_thresh"


