#!/bin/bash

#--------------------------------------------------------------------------------

# Получаем путь к место назначения из первого параметра
TARGET_PATH=$1
echo "TARGET_PATH = $TARGET_PATH"

if [ -z "$TARGET_PATH" ]
then
    # Если путь к месту назначению не задан то файлы создадим в текущей директории
    CERT_FILE_NAME=certificate.crt
    PRKEY_FILE_NAME=privateKey.key
else
    # Если путь к месту назначению задан создадим в по месту назначения
    CERT_FILE_NAME=$TARGET_PATH/certificate.crt
    PRKEY_FILE_NAME=$TARGET_PATH/privateKey.key
    # Если директория места назначения не существует то создадим её
    if [[ ! -e $TARGET_PATH ]]; then
        mkdir -p $TARGET_PATH
    fi
fi

# Удаляем старые файлы, если они существуют
rm $CERT_FILE_NAME
rm $PRKEY_FILE_NAME

#--------------------------------------------------------------------------------

# Задаём конфигурации формирования сертификата
CONFIGNAME=LinuxSslConf.cfg

HOST_NAME="example"
DOT=ru
COUNTRY=RU
STATE=LOBL
CITY=SPB
#ORGANIZATION=IT
#ORGANIZATION_UNIT=IT Department
#EMAIL=webmaster@$HOST_NAME.$DOT

#--------------------------------------------------------------------------------

# Формируем файл конфигураций
{
echo [req]
echo default_bits = 2048
echo prompt = no
echo default_md = sha256
echo x509_extensions = v3_req
echo distinguished_name = dn
echo
echo [dn]
echo C = $COUNTRY
echo ST = $STATE
echo L = $CITY
# echo O = $ORGANIZATION
# echo OU = $ORGANIZATION_UNIT
# echo emailAddress = $EMAIL
echo CN = $HOST_NAME.$DOT
echo
echo [v3_req]
echo subjectAltName = @alt_names
echo
echo [alt_names]
echo DNS.1 = *.$HOST_NAME.$DOT
echo DNS.2 = $HOST_NAME.$DOT
} >> $CONFIGNAME

#--------------------------------------------------------------------------------

# Вызываем OpenSSL для формирования сертификата и приватного ключа на основе файла конфигураций
openssl req -new -x509 -newkey rsa:2048 -sha256 -nodes -keyout $PRKEY_FILE_NAME -days 356 -out $CERT_FILE_NAME -config $CONFIGNAME
# Удаляем файл клнфигураций
rm $CONFIGNAME

#--------------------------------------------------------------------------------

