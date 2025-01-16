#ifndef CREDENTIALS_HPP
#define CREDENTIALS_HPP

// These are generated by CMakeLists.txt
// To make sure compilation succeeds, ensure ca.crt, client.crt, aws.url and client.key exist!
// https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/build-system.html#embedding-binary-data
// AWS root CA certificate
extern const uint8_t AWS_CA_CRT[]      asm("_binary_ca_crt_start");
// Device-local certificate
extern const uint8_t AWS_CLIENT_CRT[]  asm("_binary_client_crt_start");
// Device-local private key
extern const uint8_t AWS_CLIENT_KEY[]  asm("_binary_client_key_start");
// URL of AWS endpoint
extern const uint8_t AWS_URL[]         asm("_binary_aws_url_start");

#endif
