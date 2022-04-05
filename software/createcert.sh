#!/bin/bash
set -e

procSpinner() {
    spin='◢◣◤◥'                                                                                                                                                                                                                                                                  
    i=0                                                                                                                                       
    while kill -0 $1 2>/dev/null                                                                                                            
    do                                                                                                                                        
    i=$(( (i+1) %4 ))                                                                                                                       
    printf "\r${spin:$i:1}"                                                                                                                 
    sleep 0.08                                                                                                                              
    done
}

rm -f AQMca.* AQM.* cert.h key.h

openssl genrsa -out AQMca.key 4096 2> /dev/null &
procSpinner $!

cat > AQMca.conf << EOF  
[ req ]
distinguished_name     = req_distinguished_name
prompt                 = no
[ req_distinguished_name ]
C = AT
ST = CA
L = Villach
O = AETHERENGINEERING
CN = aether.local
EOF

openssl req -new -x509 -days 3650 -key AQMca.key -out AQMca.crt -config AQMca.conf  2> /dev/null
echo "01" > AQMca.srl
openssl genrsa -out AQM.key 4096 2> /dev/null

cat > AQM.conf << EOF  
[ req ]
distinguished_name     = req_distinguished_name
prompt                 = no
[ req_distinguished_name ]
C = AT
ST = AT-2
L = Villach
O = AETHERENGINEERING
CN = aether.local
EOF

openssl req -new -key AQM.key -out AQM.csr -config AQM.conf
openssl x509 -days 3650 -CA AQMca.crt -CAkey AQMca.key -in AQM.csr -req -out AQM.crt
openssl verify -CAfile AQMca.crt AQM.crt
openssl rsa -in AQM.key -outform DER -out AQM.key.DER
openssl x509 -in AQM.crt -outform DER -out AQM.crt.DER
echo "#ifndef CERT_H_" > ./cert.h
echo "#define CERT_H_" >> ./cert.h
xxd -i AQM.crt.DER >> ./cert.h
echo "#endif" >> ./cert.h
echo "#ifndef PRIVATE_KEY_H_" > ./key.h
echo "#define PRIVATE_KEY_H_" >> ./key.h
xxd -i AQM.key.DER >> ./key.h
echo "#endif" >> ./key.h
echo ""
echo "Removing temp files"
echo "---------------------"
rm -f AQMca.* AQM.*
echo "Done"
echo "moving/replacing file to/in project folder"
mv -f cert.h "include/cert.h"
mv -f key.h "include/key.h"

echo "---------------------"
echo ""
echo "Certificates created!"
echo "---------------------"
echo ""
echo "  Private key:      key.h"
echo "  Certificate data: cert.h"
echo ""