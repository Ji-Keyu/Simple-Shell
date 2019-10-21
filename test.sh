#!/bin/bash

echo abcdefg >abc.txt

touch test1out.txt
touch test1err.txt
./simpsh --rdonly abc.txt --wronly test1out.txt --wronly test1err.txt \
  --command 0 1 2 cat >c1out.txt 2>&1
T1RC=$?
sleep 1
if [ $T1RC == 0 ] && cmp -s abc.txt test1out.txt && [ ! -s test1err.txt ] \
  && [ ! -s c1out.txt ]
then
  echo "===>test case 1 passed"
else
  echo "===>test case 1 failed"
fi

touch test2out.txt
touch test2err.txt
./simpsh --rdonly abc.txt --verbose --wronly test2out.txt --wronly test2err.txt \
  --command 0 1 2 cat >c2out.txt 2>&1
T2RC=$?
sleep 1
if [ $T2RC == 0 ] && cmp -s abc.txt test2out.txt && [ ! -s test2err.txt ] \
  && [ -s c2out.txt ] && ! grep -q -- "--rdonly abc.txt" c2out.txt \
  && grep -q -- "--wronly test2out.txt" c2out.txt \
  && grep -q -- "--wronly test2err.txt" c2out.txt \
  && grep -q -- "--command 0 1 2 cat" c2out.txt
then
  echo "===>test case 2 passed"
else
  echo "===>test case 2 failed"
fi

touch test3out.txt
touch test3err.txt
touch def.txt
touch ghi.txt
./simpsh --rdonly abc.txt --wronly test2out.txt --wronly test2err.txt \
--rdonly test1out.txt --rdonly test1err.txt --rdonly c1out.txt \
--rdonly c2out.txt --rdonly test3out.txt --rdonly test3err.txt \
--rdonly def.txt --rdonly ghi.txt
T3RC=$?
sleep 1
if [ $T3RC == 0 ]
then
  echo "===>test case 3 passed"
else
  echo "===>test case 3 failed"
fi
rm *.txt