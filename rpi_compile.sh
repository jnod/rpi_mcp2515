if [ ! -d "bin" ]; then
  mkdir bin
fi

gcc -pthread *.c -lbcm2835 -lmcp2515 -o bin/rpi_mcp2515
