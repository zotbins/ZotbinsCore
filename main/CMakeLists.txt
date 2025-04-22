idf_component_register(     
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES ClientComponent FullnessComponent TaskComponent
)
                    
add_compile_definitions(
    SENSOR=1
    CAMERA=2
)
add_compile_definitions(MCU_TYPE=SENSOR)
# add_compile_definitions(MCU_TYPE=CAMERA)