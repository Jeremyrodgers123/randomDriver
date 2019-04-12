
print("Beginning Test")
#with  open("/dev/myRand","rb+") as driver:    
driver = open("/dev/myRand","rb+", buffering=0)
if driver.mode != "rb+":
    print("something went wrong with the mode")
#driver.write(b'\x00');
first = driver.read(1)
print(first)
second = driver.read(1)
print(second)
third = driver.read(1)
print(third)
fourth =  driver.read(1)
print(fourth)
print("")
driver.write(b'\x00')
first = driver.read(1)
print(first)
second = driver.read(1)
print(second)
third = driver.read(1)
print(third)
fourth =  driver.read(1)
print(fourth)

driver.close()
