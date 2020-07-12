#!/bin/bash

helpFunction()
{
   echo ""
   echo "Usage: $0 -a parameterA -b parameterB -c parameterC"
   echo -e "\t-a Description of what is parameterA"
   echo -e "\t-b Description of what is parameterB"
   exit 1 # Exit script after printing help
}

while getopts "a:b:c:" opt
do
   case "$opt" in
      a ) parameterA="$OPTARG" ;;
      b ) parameterB="$OPTARG" ;;
      ? ) helpFunction ;; # Print helpFunction in case parameter is non-existent
   esac
done

# Print helpFunction in case parameters are empty
if [ -z "$parameterA" ] || [ -z "$parameterB" ]
then
   echo "Some or all of the parameters are empty";
   helpFunction
fi

# Begin script in case all parameters are correct
echo -e "-------------------------"

echo -e "\nN: $parameterA"
echo -e "\nFileName: $parameterB"

echo -e "\n-------------------------"

for i in $(seq 1 $parameterA); do

   #echo -e " \033[1;33m Amarelo \033[0m"
   #echo -e " \033[1;34m Azul  Claro \033[0m"
   #echo -e " \033[1;35m Purple Claro  \033[0m"
   #echo -e " \033[5;32m Piscando    \033[0m"

	echo -e "\n========================== \033[1;33m INICIANDO PROGRAMA ... ITERAÇÃO $i \033[0m=============================\n"
	./$parameterB > log$i.txt

done
