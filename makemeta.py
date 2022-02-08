import os
from re import X
import time

x = time.time()
y = os.system(r'.\build\tests\test_exe tests/test.sf')
print('---------------------')
print(y)
print(time.time() - x)