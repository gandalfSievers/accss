#!/bin/bash

ecode=0

testcss()
{
    file=$1;
    compat=${file#*/}
    compat=${compat#*.}
    compat=${compat%%.*}
    ref=${file/src/res}
    ref=${ref%.*}.res

    if [ -e $ref ]
    then
        ac=$( ./src/accss --compat $compat $file)
        code=$?
        txt=$(echo $ac | diff -u --ignore-space-change - $ref)

        if [ $code -eq 0 ]
        then
            if [ "$txt" != "" ]
            then
                echo "Test: $file failed"
                echo -e "$txt"
                ecode=1
            else
                echo "Test: $file OK"
            fi
        else
            echo "Test: $file failed"
            echo "accss exited with "$code
            ecode=$code
        fi
    else
        echo "Test: $file"
        echo "Expected result $ref not found"
        ecode=1
    fi

}


for file in ./tests/compat/src/*.css
do
    testcss $file
done

if [ $ecode -ne 0 ]
then
    exit 1
fi
