::--------------------------------------------------------------------------------

:: Получаем путь к место назначения из первого параметра
@SET TARGET_PATH=%1

@IF "%TARGET_PATH%" == "" (
    :: Если путь к месту назначению не задан то файлы создадим в текущей директории
	SET CERT_FILE_NAME=certificate.crt
	SET PRKEY_FILE_NAME=privateKey.key
) ELSE (
    :: Если путь к месту назначению задан создадим в по месту назначения
    SET CERT_FILE_NAME=%TARGET_PATH%/certificate.crt
    SET PRKEY_FILE_NAME=%$TARGET_PATH%/privateKey.key
    :: Если директория места назначения не существует то создадим её
    IF NOT EXIST "%TARGET_PATH%" (
        mkdir -p "%TARGET_PATH%"
    )
)

:: Удаляем старые файлы, если они существуют
del %PRKEY_FILE_NAME%
del %CERT_FILE_NAME%

::--------------------------------------------------------------------------------

:: Задаём конфигурации формирования сертификата
@SET CONFIGNAME=WinSslConf.cfg

@SET HOSTNAME=example
@SET DOT=ru
@SET COUNTRY=RU
@SET STATE=LOBL
@SET CITY=SPB
:: SET ORGANIZATION=IT
:: SET ORGANIZATION_UNIT=IT Department
:: SET EMAIL=webmaster@%HOSTNAME%.%DOT%

::--------------------------------------------------------------------------------

:: Формируем файл конфигураций
@(
@echo [req]
@echo default_bits = 2048
@echo prompt = no
@echo default_md = sha256
@echo x509_extensions = v3_req
@echo distinguished_name = dn
@echo:
@echo [dn]
@echo C = %COUNTRY%
@echo ST = %STATE%
@echo L = %CITY%
:: echo O = %ORGANIZATION%
:: echo OU = %ORGANIZATION_UNIT%
:: echo emailAddress = %EMAIL%
@echo CN = %HOSTNAME%.%DOT%
@echo:
@echo [v3_req]
@echo subjectAltName = @alt_names
@echo:
@echo [alt_names]
@echo DNS.1 = *.%HOSTNAME%.%DOT%
@echo DNS.2 = %HOSTNAME%.%DOT%
)>%CONFIGNAME%

::--------------------------------------------------------------------------------

:: Вызываем OpenSSL для формирования сертификата и приватного ключа на основе файла конфигураций
openssl req -new -x509 -newkey rsa:2048 -sha256 -nodes -keyout %PRKEY_FILE_NAME% -days 356 -out %CERT_FILE_NAME% -config %CONFIGNAME%
:: Удаляем файл клнфигураций
del %CONFIGNAME%

::--------------------------------------------------------------------------------
