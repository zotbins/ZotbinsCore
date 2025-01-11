#ifndef CREDENTIALS_HPP
#define CREDENTIALS_HPP

const char* AWS_IOT_ENDPOINT = ""; // mqtts:// link :8883

// Amazon Root CA 1
static const char AWS_CERT_CA[] = R"EOF(-----BEGIN CERTIFICATE-----
insert here
-----END CERTIFICATE-----)EOF";
 
// Device Certificate                                               //change this
static const char AWS_CERT_CRT[] = R"KEY(-----BEGIN CERTIFICATE-----
insert here
-----END CERTIFICATE-----)KEY";
 
// Device Private Key                                               //change this
static const char AWS_CERT_PRIVATE[] = R"KEY(-----BEGIN RSA PRIVATE KEY-----
insert here
-----END RSA PRIVATE KEY-----)KEY";

#endif
