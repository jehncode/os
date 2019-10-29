# imports
import string
import random

# variables
filenames = ["bboozler", "inkbrush", "splashot"]

# generates and returns a string of n random characters
def getRandAlphaString(n): 
  # start with empty string
  randomstr = ""

  # generate and append string with characters
  for i in range(0, n): 
    # note ascii_letters is all lowercase and uppercase alphas
    randomstr += random.choice(string.ascii_letters);

  return randomstr


# start of program

# create files containing random string
for name in filenames:
  # open/create file with name of 'name' with write permission
  wfile = open(name, "w")  
  # generate random string
  randstr = getRandAlphaString(10)
  # append file with random string + new line
  wfile.write(randstr + '\n')
  # close file
  wfile.close()
  # print string to screen
  print(randstr)

# print two random integers with range from 1 to 42
minval = 1
maxval = 42
rand1 = random.randint(minval, maxval)
rand2 = random.randint(minval, maxval)
print rand1
print rand2

# print product of the two random integers
prod = rand1 * rand2
print prod
# end of program
