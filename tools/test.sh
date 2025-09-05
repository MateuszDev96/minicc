#!/bin/bash

declare -a test_cases=(
  # "program1:0"
  # "program2:42"
  # "program3:114"
  # "program4:41"
  # "program5:7"
  # "program6:9"
  # "program7:70"
  # "program8:4"
  # "program9:1"
  # "program10:1"
  # "program11:2"
  # "program12:0"
  # "program13:1"
  # "program14:1"
  # "program15:0"
  # "program16:1"
  # "program17:0"
  # "program18:0"
  # "program19:1"
  # "program20:1"
  # "program21:0"
  # "program22:1"
  # "program23:0"
  # "program24:0"
  # "program25:1"
  # "program26:1"
  # "program27:0"
  # "program28:3"
  # "program29:3"
  # "program30:3"
  # "program31:8"
  # "program32:1"
  # "program33:2"
  # "program34:3"
  # "program35:8"
  # "program36:6"
  # "program37:3"
  # "program38:8"
  # "program39:3"
  # "program40:3"
  # "program41:2"
  # "program42:2"
  # "program43:55"
  # "program44:3"
  # "program44:3"
  # "program45:10"
  # "program46:3"
  # "program47:5"
  # "program48:10"
  # "program49:55"
  # "program50:3"
  # "program51:3"
  # "program52:5"
  # "program53:3"
  # "program54:5"
  # "program55:5"
  # "program56:7"
  # "program57:7"
  # "program58:5"
  # "program59:8"
  # "program60:8"

  # # # pointers
  # "program61:3"
  # "program62:3"
  # "program63:4"
  # "program64:5"
  # "program65:0"
  # "program66:1"
  # "program67:2"
  # "program68:3"
  # "program69:4"
  # "program70:5"
  # "program71:0"
  # "program72:1"
  # "program73:2"
  # "program74:3"
  # "program75:4"
  # "program76:5"
  # "program77:8"
  # "program78:8"
  # "program79:8"
  # "program80:32"
  # "program81:96"
  # "program82:32"
  # "program83:8"
  # "program84:9"
  # "program85:9"
  # "program86:8"
  # "program87:8"
  # "program88:1"

  # "program89:3"
  # "program90:5"
  # "program91:8"
  # "program92:2"
  "program93:21"
  # "program94:66"
  # "program95:136"
  # "program96:32"
  # "program97:7"
  # "program98:1"
  # "program99:55"
)

for entry in "${test_cases[@]}"; do
  prog="${entry%%:*}"             # Nazwa programu (np. program1)
  expected="${entry##*:}"         # Oczekiwany kod wyjścia (np. 0)
  path="./tests/$prog"

  if [ ! -d "$path" ]; then
    echo "POMINIĘTO: brak katalogu $path"
    continue
  fi

  ./build/minicc "$path/$prog.cwe" "$path/$prog.s"
  if [ $? -ne 0 ]; then
    echo "❌ FAIL - błąd kompilacji"
    exit 1
  fi

  riscv64-linux-gnu-gcc -static -o "$path/$prog.exec" "$path/$prog.s"
  if [ $? -ne 0 ]; then
    echo "❌ FAIL - błąd linkowania"
    exit 1
  fi

  qemu-riscv64 "$path/$prog.exec"
  actual_exit_code=$?

  if [ "$actual_exit_code" = "$expected" ]; then
    echo "✅ OK - Test $prog zakończony pomyślnie"
  else
    echo "❌ FAIL - Test $prog nie powiódł się"
    exit 1
  fi
done
