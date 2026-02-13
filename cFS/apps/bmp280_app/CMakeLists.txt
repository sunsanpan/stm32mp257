##################################################################
#
# BMP280 Application CMakeLists.txt
#
##################################################################

project(BMP280_APP C)

# Create the app module
add_cfe_app(bmp280_app 
    fsw/src/bmp280_app.c
)

# Add include directories
target_include_directories(bmp280_app PUBLIC
    fsw/src
)

# This application needs no additional libraries
