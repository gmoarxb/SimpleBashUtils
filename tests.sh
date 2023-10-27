#!/bin/bash

test_number=0
start_number=$2

compare() {
    echo "Test number: $test_number"
    cat $1 tf1 tf2 tf3 tf4 > cat_result
    ./s21_cat $1 tf1 tf2 tf3 tf4 > s21_result
    echo -e "$(diff -s cat_result s21_result)\n"
    if [[ $(diff -q cat_result s21_result) ]]
    then
        echo "The wrong combination of options is \"$1\""
        exit
    fi
    test_number=$((test_number + 1))
}

options=(-A -b -e -E -n -s -t -T -v)

while [ $start_number != 0 ]; do

gcc geberate.c -o geb.out
./geb.out tf1 tf2 tf3 tf4

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

if [[ $1 -eq 4 ]]; then 
for opt1 in ${options[@]}
do
    for opt2 in ${options[@]}
    do
        for opt3 in ${options[@]}
        do
            for opt4 in ${options[@]}
            do
                if [ $opt1 != $opt2 ] && [ $opt1 != $opt3 ] && [ $opt1 != $opt4 ] && \
                [ $opt2 != $opt3 ] && [ $opt2 != $opt4 ] && \
                [ $opt3 != $opt4 ]
                then
                    opts=$opt1" "$opt2" "$opt3" "$opt4
                    compare $opts
                fi
            done
        done
    done
done
fi
start_number=$((start_number - 1))
rm tf1 tf2 tf3 tf4 cat_result s21_result geb.out
done