@SET TARGET_PATH=%1

@IF "%TARGET_PATH%" == "" (
	@SET CERT_FILE_NAME=certificate.crt
	@SET PRKEY_FILE_NAME=privateKey.key
) ELSE (
    @SET CERT_FILE_NAME=%TARGET_PATH%/certificate.crt
    @SET PRKEY_FILE_NAME=%$TARGET_PATH%/privateKey.key
)

del %PRKEY_FILE_NAME%
del %CERT_FILE_NAME%

@SET CONFIGNAME=WinSslConf.cfg

@SET HOSTNAME=example
@SET DOT=ru
@SET COUNTRY=RU
@SET STATE=LOBL
@SET CITY=SPB
@REM SET ORGANIZATION=IT
@REM SET ORGANIZATION_UNIT=IT Department
@REM SET EMAIL=webmaster@%HOSTNAME%.%DOT%

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
@REM echo O = %ORGANIZATION%
@REM echo OU = %ORGANIZATION_UNIT%
@REM echo emailAddress = %EMAIL%
@echo CN = %HOSTNAME%.%DOT%
@echo:
@echo [v3_req]
@echo subjectAltName = @alt_names
@echo:
@echo [alt_names]
@echo DNS.1 = *.%HOSTNAME%.%DOT%
@echo DNS.2 = %HOSTNAME%.%DOT%
)>%CONFIGNAME%

openssl req -new -x509 -newkey rsa:2048 -sha256 -nodes -keyout %PRKEY_FILE_NAME% -days 356 -out %CERT_FILE_NAME% -config %CONFIGNAME%

del %CONFIGNAME%