#!/bin/bash

#task=( "resnet18_cifar10" "resnet18_cifar100" "lstm_wikitext2" "ncf_ml-20m" )
#reducer=( "sage" "thresh" "topk" "exact" )
#job=()
#for t in ${task[@]}
#do
#	for r in ${reducer[@]}
#	do
#		job+=($t"_"$r)
#	done
#done 

dir="./results"

for entry in $dir/*
do
	python3 parser.py --job "$entry"
	echo Done: "$entry"
done
