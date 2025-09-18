idf_component_register(SRCS "peripheral_manager.cpp"
                       INCLUDE_DIRS "include"
                       REQUIRES log freertos
                       PRIV_REQUIRES peripheral_interface peripheral_queue publish)