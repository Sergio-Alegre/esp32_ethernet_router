idf_component_register(SRCS "esp32_nat_router.c"
                            "portmap.c"
                            "dhcp_manager.c"
                            "vpn_manager.c"
                            "netif_hooks.c"
                            "led_strip_status.c"
                    INCLUDE_DIRS "."
                    PRIV_REQUIRES pcap_capture esp_wifi esp_netif console nvs_flash driver esp_http_server fatfs cmd_router dhcpserver wpa_supplicant cmd_system acl remote_console syslog json esp_wireguard esp_eth http_server app_update led_strip eth_w5500 esp_system mqtt_ha espressif__mdns netflow)

# Performance optimization for the hot-path packet processing code
target_compile_options(${COMPONENT_LIB} PRIVATE -O2)
# Setup and event-handling files build for size; netif_hooks.c stays on -O2
# because it sits on the packet path. Source-level COMPILE_OPTIONS are emitted
# after the target options, so the later -Os wins for the listed files.
set_source_files_properties(esp32_nat_router.c portmap.c dhcp_manager.c vpn_manager.c led_strip_status.c
    PROPERTIES COMPILE_OPTIONS "-Os")
