testcss()
{
    file=$1;
    compat=${file#*.}
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
                exit 1
            fi
        else
            echo "Test: $file failed"
            echo "accss exited with "$code
            exit $code
        fi
    else
        echo "Test: $file"
        echo "Expected result $ref not found"
        exit 1
    fi

}


for file in ./tests/compat/src/*.css
do
    testcss $file
done
