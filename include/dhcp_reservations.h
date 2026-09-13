/* DHCP reservation structs, connected-client helpers, and lease queries.
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "router_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DHCP_RESERVATIONS (AP_MAX_CONNECTIONS + 2)
#define DHCP_RESERVATION_NAME_LEN 32

/* Isolation levels for a reservation (enforced by source MAC in the ETH input hook) */
#define DHCP_ISOLATION_OFF 0
#define DHCP_ISOLATION_LAN 1   /* Internet only: no private ranges, multicast/broadcast or router services */

struct dhcp_reservation_entry {
    uint8_t mac[6];
    uint32_t ip;
    char name[DHCP_RESERVATION_NAME_LEN];
    uint8_t valid;
    uint8_t isolation;   /* DHCP_ISOLATION_*; uses former tail padding, so the NVS blob size is unchanged */
};

#ifndef __cplusplus
/* The "dhcp_res" NVS blob is length-checked on load; growing the struct would discard stored reservations */
_Static_assert(sizeof(struct dhcp_reservation_entry) == 48, "dhcp_reservation_entry size changed (NVS blob layout)");
#endif

/**
 * @brief Information about a connected client
 */
typedef struct {
    uint8_t mac[6];                          /**< Client MAC address */
    uint32_t ip;                             /**< Client IP (0 if unknown) */
    char name[DHCP_RESERVATION_NAME_LEN];    /**< Device name from reservation (empty if none) */
    bool has_ip;                             /**< True if IP was found in DHCP leases */
} connected_client_t;

/**
 * @brief Structure for DHCP lease information (from custom dhcpserver)
 * @note Defined here to avoid header conflicts with ESP-IDF's built-in dhcpserver.h
 */
typedef struct {
    uint8_t mac[6];       /**< Client MAC address */
    uint32_t ip;          /**< Client IP address (network byte order) */
    uint32_t lease_timer; /**< Remaining lease time in seconds */
    char hostname[DHCP_RESERVATION_NAME_LEN]; /**< Client hostname from DHCP Option 12 */
} dhcp_lease_info_t;

extern struct dhcp_reservation_entry dhcp_reservations[];

esp_err_t get_dhcp_reservations(void);
void print_dhcp_reservations(void);
esp_err_t add_dhcp_reservation(const uint8_t *mac, uint32_t ip, const char *name);
esp_err_t del_dhcp_reservation(const uint8_t *mac);
esp_err_t clear_all_dhcp_reservations(void);
uint32_t lookup_dhcp_reservation(const uint8_t *mac);
bool is_ip_reserved_for_other(uint32_t ip, const uint8_t *mac);

/**
 * @brief Set the isolation level of an existing reservation and persist it
 * @param mac MAC address of the reservation (6 bytes)
 * @param level DHCP_ISOLATION_OFF or DHCP_ISOLATION_LAN
 * @return ESP_OK, ESP_ERR_NOT_FOUND if no reservation for the MAC, or NVS error
 */
esp_err_t set_dhcp_reservation_isolation(const uint8_t *mac, uint8_t level);

/**
 * @brief Number of valid reservations with isolation enabled (hot-path fast check)
 */
int dhcp_isolated_count(void);

/**
 * @brief Packets dropped by LAN isolation for a reservation slot (RAM only, reset on boot)
 * @param idx Index into dhcp_reservations[]
 */
uint32_t get_isolation_drops(int idx);

/**
 * @brief Look up device name by IP address from DHCP reservations
 * @param ip IP address to look up (network byte order)
 * @return Device name if found, NULL if no reservation with that IP
 */
const char* lookup_device_name_by_ip(uint32_t ip);

/**
 * @brief Look up device name by MAC address from DHCP reservations
 * @param mac MAC address to look up (6 bytes)
 * @return Device name if found, NULL if no reservation with that MAC
 */
const char* lookup_device_name_by_mac(const uint8_t *mac);

/**
 * @brief Resolve a device name to an IP address from DHCP reservations
 * @param name Device name to look up (case-insensitive)
 * @param ip Output IP address (network byte order)
 * @return true if found, false if no reservation with that name
 */
bool resolve_device_name_to_ip(const char *name, uint32_t *ip);

/**
 * @brief Resolve a device name to a MAC address from DHCP reservations
 * @param name Device name to look up (case-insensitive)
 * @param mac Output MAC address (6 bytes)
 * @return true if found, false if no reservation with that name
 */
bool resolve_device_name_to_mac(const char *name, uint8_t mac[6]);

void get_dhcp_pool_range(uint32_t server_ip, uint32_t *start_ip, uint32_t *end_ip);
void print_dhcp_pool(void);

/**
 * @brief Get list of currently connected WiFi clients
 * @param clients Array to store client information
 * @param max_clients Maximum number of clients to return
 * @return Number of connected clients found
 */
int get_connected_clients(connected_client_t *clients, int max_clients);

/**
 * @brief Enumerate all active DHCP leases
 * @param leases Array to store lease information
 * @param max_leases Maximum number of leases to return
 * @return Number of active leases found (0 if DHCP server not running)
 */
int dhcps_get_active_leases(dhcp_lease_info_t *leases, int max_leases);

#ifdef __cplusplus
}
#endif
