# Only include AWS credentials if all files are present.
if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/credentials/client.key"
    AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/credentials/client.crt"
    AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/credentials/ca.crt"
    AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/credentials/aws.url"
)
    message("Adding AWS key components...")
    set(HAS_AWS_CREDENTIALS TRUE)
    set(AWS_EMBED
        credentials/client.key
        credentials/client.crt
        credentials/ca.crt
        credentials/aws.url
    )
else()
    message(WARNING "Not all AWS key components found, adding polyfill...")
    set(HAS_AWS_CREDENTIALS FALSE)
    set(AWS_EMBED "")
endif()

idf_component_register(SRCS "client_publish.cpp" "client_connect.cpp"
                    INCLUDE_DIRS "include"
                    EMBED_TXTFILES ${AWS_EMBED}
                    REQUIRES mqtt esp_system esp_event esp_netif nvs_flash protocol_examples_common
                    )
if(HAS_AWS_CREDENTIALS)
    target_compile_definitions("${COMPONENT_TARGET}" PUBLIC HAS_AWS_CREDENTIALS)
endif()
