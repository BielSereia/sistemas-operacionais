import os
import time

data = ["x" * 100 for _ in range(1_000_000)]
print("Criado:", len(data), "itens", os.getpid())

while True:
    time.sleep(1)