import serial
import struct
import numpy as np
import matplotlib.pyplot as plt
import time

PORTA = "COM6"
BAUD = 115200

CMD_RECEIVE_INT = 1

N = 50

#
# gera senoide
#
fs = 10000.0   # taxa de atualização
f  = 600.0      # senoide desejada

t = np.arange(N)

theta = 2*np.pi*f*t/fs

onda = 2048 + 1800*np.sin(theta)

onda = onda.astype(np.uint16)

#
# abre serial
#
ser = serial.Serial(PORTA,BAUD,timeout=5)

#
# limpa buffers serial
#
ser.reset_input_buffer()
ser.reset_output_buffer()

#
# cabeçalho
#
header = struct.pack("<BH",CMD_RECEIVE_INT,N*2)

print("Enviando vetor...")

#
# envia header
#
ser.write(header)

time.sleep(0.05)

#
# envia amostras
#
for valor in onda:

    ser.write(struct.pack("<H",int(valor)))

    #
    # pequeno delay
    #
    time.sleep(0.001)

print("Aguardando retorno ADC...")

dados = []

#
# recebe ADC
#
for i in range(N):

    rx = ser.read(2)

    if len(rx) < 2:
        print("Timeout")
        break

    valor = struct.unpack("<H", rx)[0]

    dados.append(valor)

ser.close()

dados = np.array(dados)

#
# proteção FFT
#
if len(dados) == 0:
    print("Nenhum dado recebido")
    exit()

#
# FFT
#
fft_adc = np.abs(
    np.fft.rfft(dados)
)

freq = np.fft.rfftfreq(len(dados),d=1/fs)

#
# gráficos
#
plt.figure(figsize=(10,8))

#
# domínio do tempo
#
plt.subplot(2,1,1)

plt.plot(t,onda,label="Sinal enviado")

plt.plot(t,dados,label="Sinal recebido")

plt.title("Dominio do tempo")

plt.xlabel("Amostras")

plt.ylabel("Amplitude")

plt.grid(True)

plt.legend()

#
# FFT
#
plt.subplot(2,1,2)

plt.plot(freq,fft_adc)

plt.title("FFT do sinal recebido")

plt.xlabel("Frequência [Hz]")

plt.ylabel("Magnitude")

plt.grid(True)

plt.tight_layout()

plt.show()