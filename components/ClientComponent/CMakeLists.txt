# target_compile_options(Client.cpp PRIVATE -fpermissive)

# only include the aws credential files if they exist
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/client.key"
    AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/client.crt"
    AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/ca.crt"
    AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/aws.url"
)
    message("Adding AWS key components")
    set(HAS_AWS_CREDENTIALS TRUE)
    set(AWS_EMBED client.key client.crt ca.crt aws.url)
else()
    message(WARNING "Not all AWS key components found, adding polyfill")
    set(HAS_AWS_CREDENTIALS FALSE)
    set(AWS_EMBED "")
endif()

idf_component_register(SRCS "Client.cpp" "Serialize.cpp"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES log esp_system esp_partition nvs_flash esp_event esp_netif protocol_examples_common mqtt app_update json esp_wifi
                    EMBED_TXTFILES "${AWS_EMBED}")
if(HAS_AWS_CREDENTIALS)
    target_compile_definitions("${COMPONENT_TARGET}" PRIVATE HAS_AWS_CREDENTIALS)
endif()
