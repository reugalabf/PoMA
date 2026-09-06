idf_component_register(SRCS "src/poma_tcpconnector.c" "src/poma_bleconnector.c" "src/poma_core.c"
                    INCLUDE_DIRS "include"
                    REQUIRES lwip bt nvs_flash)
