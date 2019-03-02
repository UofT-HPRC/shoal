# Set the reference directory for source file relative paths
set origin_dir ${::env(SHOAL_PATH)}/GAScore/vivado
set vivado_ver ${::env(SHOAL_VIVADO_VERSION)}
set part_name ${::env(SHOAL_PART)}

# Set the directory path for the original project from where this script was exported
set proj_dir "[file normalize "$origin_dir/projects/$vivado_ver/$part_name/MB_GAScore_1"]"
set sdk_dir $proj_dir/MB_GAScore_1.sdk

# Set SDK workspace
setws $sdk_dir
# Create a HW project
createhw -name hw_platform_0 -hwspec $sdk_dir/MB_GAScore_1_bd_wrapper.hdf
# Create a BSP project
createbsp -name bsp_0 -hwproject hw_platform_0 -proc microblaze_0 -os standalone
createbsp -name bsp_1 -hwproject hw_platform_0 -proc microblaze_1 -os standalone
# Create application project
createapp -name MB_0 -hwproject hw_platform_0 -bsp bsp_0 -proc microblaze_0 \
-os standalone -lang C++ -app {Empty Application}
createapp -name MB_1 -hwproject hw_platform_0 -bsp bsp_1 -proc microblaze_1 \
-os standalone -lang C++ -app {Empty Application}
# Add include paths
configapp -app MB_0 -add include-path "${::env(SHOAL_PATH)}/include"
configapp -app MB_1 -add include-path "${::env(SHOAL_PATH)}/include"

# Copy source code
file copy -force -- ${::env(SHOAL_PATH)}/GAScore/testbench/MB_GAScore_1_MB0.cpp $sdk_dir/MB_0/src/main.cc
file copy -force -- ${::env(SHOAL_PATH)}/GAScore/testbench/MB_GAScore_1_MB1.cpp $sdk_dir/MB_1/src/main.cc

# Build all projects
projects -build
