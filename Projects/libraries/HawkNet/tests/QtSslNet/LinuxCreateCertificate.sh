#!/usr/bin/expect -f

set timeout -1

set TARGET_PATH [lindex $argv 0]

#Если путь к месту назначению пуст то файлы создадим в текущей директории
if {$TARGET_PATH eq ""} {
    set CERT_FILE_NAME certificate.crt
    set PRKEY_FILE_NAME privateKey.key
} else {
    set CERT_FILE_NAME $TARGET_PATH/certificate.crt
    set PRKEY_FILE_NAME $TARGET_PATH/privateKey.key
}

#---------------------------

spawn rm $CERT_FILE_NAME
spawn rm $PRKEY_FILE_NAME

spawn openssl req -x509 -sha256 -nodes -days 365 -newkey rsa:2048 -keyout $PRKEY_FILE_NAME -out $CERT_FILE_NAME

expect "Country Name (2 letter code)"
send -- "RU\r"

expect "State or Province Name (full name)"
send -- "\r"

expect "Locality Name (eg, city)"
send -- "SPB\r"

expect "Organization Name (eg, company)"
send -- "\r"

expect "Organizational Unit Name (eg, section)"
send -- "\r"

expect "Common Name (e.g. server FQDN or YOUR name)"
send -- "BE_GET\r"

expect "Email Address"
send -- "\r"

expect eof
