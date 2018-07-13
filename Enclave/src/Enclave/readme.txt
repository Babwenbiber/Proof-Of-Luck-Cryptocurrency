Am besten erstmal eine locale kopie vom src ordner zum Testen anlegen und darin dann:

1. Ordner names build erstellen
2. terminal in Ordner öffnen
3. cmake ..
4. make app
5. make enclave
6. libenclave.signed.so umbenennen zu enclave.signed.so
7. ./app


cmake setzt automatisch das lib davor, wir werden die enclavefile in der app noch zu libenclave umbenennen.
