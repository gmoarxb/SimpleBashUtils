#!/bin/bash

if [[ ! -s ./s21_cat ]]; then
    echo "You must put cat_test.sh and cat_geberate.c"
    echo  "to one directory with your s21_cat executable file"
    exit
fi


mkdir -p test_directory
cp ./s21_cat test_directory/
cd test_directory/

test_number=0
start_number=$(( 64 / $1 / $1))
#start_number=$(( 64 / $1 ))

files_list="random_1 empty random_2 lfd random_3 random_4"

compare() {
    echo "Test number: $test_number"
    ./s21_cat $1 $files_list > s21_result
    cat $1 $files_list > cat_result
    echo -e "$(diff -s cat_result s21_result)\n"
    if [[ $(diff -q cat_result s21_result) ]]
    then
        echo "____________________________________________________________"
        echo -e "\nThe wrong combination of options is \n\t$1\n"
        echo -e "The test files in ./test_directory:"
        echo -e "\t$files_list"

        echo -e "\n\nSo to repeat this test go to ./test_directory and run"
        echo -e "\tcat $1 $files_list > cat_result"
        echo "vs"
        echo -e "\t./s21_cat $1 $files_list > s21_result"
        echo -e "\nand compare their results by vscode or smth like it"
        exit
    fi
    test_number=$((test_number + 1))
    
}

options=(-A -b -e -E -n -s -t -T -v)
gcc ../cat_geberate.c -o geb.out

while [ $start_number != 0 ]; do

start_number=$((start_number - 1))
./geb.out $files_list

if [[ $1 -eq 1 ]]; then
for opt1 in ${options[@]}
do
    opts=$opt1
    compare $opts
done
fi

if [[ $1 -eq 2 ]]; then
for opt1 in ${options[@]}
do
    for opt2 in ${options[@]}
    do
        if [ $opt1 != $opt2 ]
        then
            opts=$opt1" "$opt2
            compare $opts
        fi
    done
done
fi

if [[ $1 -eq 3 ]]; then
for opt1 in ${options[@]}
do
    for opt2 in ${options[@]}
    do
        for opt3 in ${options[@]}
        do
            if [ $opt1 != $opt2 ] && [ $opt1 != $opt3 ] && \
            [ $opt2 != $opt3 ]
            then
                opts=$opt1" "$opt2" "$opt3
                compare $opts
            fi
        done
    done
done
fi

done

cd ..
rm -rf ./test_directory