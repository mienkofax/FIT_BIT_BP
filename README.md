# Bakalárska práca

Author: Peter Tisovčík<xtisov00@stud.fit.vutbr.cz>

[Last version of BeeOn Gateway](https://github.com/BeeeOn/gateway)

----------------------------------------------------------

## Štruktúra priložených súborov

 * README
 * [CREATED](CREATED.md) - zoznam súborov v rámci bakalárskej práce
 * tex - zdrojové súbory bakalárskej práce
 * src - vytvorené súbory v rámci bakalárksej práce (nepreložiteľné)
 * original - zdrojové kódy celej BeeeOn Gateway aplikácie
 * xtisov00.pdf - text bakalárskej práce

----------------------------------------------------------

## Potrebné knižnice pre kompiláciu zdrojových súborov
 * libpoco - https://pocoproject.org/
 * openzwave - https://github.com/OpenZWave/open-zwave

----------------------------------------------------------

## Kompilácia zdrojových súborov
```
mkdir build
cd build
cmake ..
make -j`nproc`
```

----------------------------------------------------------

### Spustenie BeeeOn Gateway aplikácie
```
./build/src/beeeon-gateway -c conf/gateway-startup.ini
```

### Spustenie manažéra zariadení Jablotron
```
./build/src/beeeon-gateway -c conf/jablotron-startup.ini
```

### Spustenie manažéra zariadení Z-Wave
```
./build/src/beeeon-gateway -c conf/z-wave-startup.ini
```

----------------------------------------------------------

## Testovanie - Jednotkové testy

### Spustenie jednotkových testov:
```
./build/test/test-suite-gateway
```

----------------------------------------------------------

## Testovanie - Funkčné testy

Spustenie testovacej BeeeOn Gateway aplikácie: `./build/src/beeeon-gateway -c conf/test-register-device-manager.ini`

1. Registrovanie manažéra zariadení:
	```
	./build/src/beeeon-gateway -c conf/test-register-device-manager.ini
	./build/src/beeeon-gateway -c conf/jablotron-startup.ini
	```

2. Zoznam napárovaných zariadení:
	```
	./build/src/beeeon-gateway -c conf/test-device-list.ini
	./build/src/beeeon-gateway -c conf/z-wave-startup.ini
	```

3. Posledná nameraná hodnota
	```
	./build/src/beeeon-gateway -c conf/last-value.ini
	./build/src/beeeon-gateway -c conf/jablotron-startup.ini
	```

4. Pridanie nového zariadenia
	```
	./build/src/beeeon-gateway -c conf/test-listen.ini
	./build/src/beeeon-gateway -c conf/z-wave-startup.ini
	```

5. Odpárovanie pridaného zariadenia
	```
	./build/src/beeeon-gateway -c conf/test-unpair.ini
	./build/src/beeeon-gateway -c conf/jablotron-startup.ini
	```

6. Nastavenie hodnoty na senzore
	```
	./build/src/beeeon-gateway -c conf/test-set-value.ini
	./build/src/beeeon-gateway -c conf/jablotron-startup.ini
	```
