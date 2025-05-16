idf_component_register(     
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES ClientComponent FullnessComponent TaskComponent
)

# you also need to change this compile definition in the taskcomponent and clientcomponent cmakelists.txt
                    
# add_compile_definitions(SENSOR)
add_compile_definitions(CAMERA)