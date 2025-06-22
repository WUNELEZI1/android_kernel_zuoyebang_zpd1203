#!/bin/bash
#-------------------------------------------------------------------------------
#-                                                                            --
#-       This software is confidential and proprietary and may be used        --
#-        only as expressly authorized by a licensing agreement from          --
#-                                                                            --
#-                            VeriSilicon Inc.                                --
#-                                                                            --
#-                   (C) COPYRIGHT 2015 VeriSilicon Inc                       --
#-                            ALL RIGHTS RESERVED                             --
#-                                                                            --
#-                 The entire notice above must be reproduced                 --
#-                  on all copies and should not be removed.                  --
#-                                                                            --
#-------------------------------------------------------------------------------

#Description: Creates code for various purposes from
#             given swhw register definition

if [ -z "$1" ] || [ ! -e "$1" ]
then
        echo " This script produces vcmdswregisters.h, vcmdswregisters_directives.tcl,"
        echo " hwdriver.vhd, signals.vhd, vcmdregistertable.h and vcmdregisterenum.h files"
        echo " from given .csv register description"
        echo ""
        echo "Usage: ./create_vcmdswreg_struct.sh 'fileName.csv' "
        exit
fi

fileName=$1
version=`echo $fileName | cut -d '_' -f 2`

if [ "$version" == "" ]
then
  echo "Document $1 version not found. Exit..."
  exit
else
  echo "Creating vcmdswregister struct from $1 version $version"
fi

fileTmp="vcmdregister.csv"
totalFields=9
regNum=31

if [ -e $fileTmp ]; then
        rm $fileTmp
fi

chmod 777 $fileName
dos2unix -q $fileName

fileLicense="driver_license.txt"
if [ -e $fileLicense ]; then
        cp $fileLicense vcmdswregisters.h
        cp $fileLicense vcmdregistertable.h
        cp $fileLicense vcmdregisterenum.h
fi

while read line; do
        echo $line | sed 's/\s,/,/g' | sed 's/"//g' | sed 's/?//g' | sed 's/_lsb//g' | sed 's/W1C/RW/g' | awk -F ',' '
        {
                if ($1 ~ /^[0-9]+$/) {
                        printf "%s", $0 >> "vcmdregister.csv"
                        for (i = 1; i <= NF; i++) {
                                if ($(i) == "RO" || $(i) == "RW" || $(i) == "WO") {
                                        printf "\n" >> "vcmdregister.csv"
                                        break;
                                }
                        }
                } else {
                        for (i = 1; i <= NF; i++) {
                                if ($(i) == "RO" || $(i) == "RW" || $(i) == "WO") {
                                        printf ",%s,%s,%s,%s\n", $(i), $(i+1), $(i+2), $(i+3) >> "vcmdregister.csv"
                                        break;
                                }
                        }
                }
        }
        '
done < $fileName

catapultPrefix=SwRegister_
catapultPostfix=""   ##empty (if interface label to direct input made correctly


awk '
        BEGIN{FS=",";   printf "\n\n">> "vcmdswregisters.h";
                        printf "//Description: Common SWHW interface structure definition\n">> "vcmdswregisters.h";
                        printf "//Based on document version '$version'\n">> "vcmdswregisters.h";
                        printf "#ifndef VCMD_SWREGISTERS_H_\n">> "vcmdswregisters.h";
                        printf "#define VCMD_SWREGISTERS_H_\n\n">> "vcmdswregisters.h";
                        printf "#include \"actypes.h\"\n\n">> "vcmdswregisters.h";
                        printf "struct VcmdSwRegisters {\n">> "vcmdswregisters.h";
                        printf"--signal declaration from Document Version '$version'\n" > "signals.vhd";
                        printf"--register to signal map table from Document Version '$version'\n" > "hwdriver.vhd"
                        printf"##Common catapult directives from Document Version '$version' \n" > "vcmdswregisters_directives.tcl"
                        printf "\n\n">> "vcmdregisterenum.h";
                        printf "/* Register interface based on the document version '$version' */\n">> "vcmdregisterenum.h";
                        printf "\n\n">> "vcmdregistertable.h";
                        printf "/* Register interface based on the document version '$version' */\n">> "vcmdregistertable.h";
        }
        END{printf "};\n\n#endif /*VCMD_SWREGISTERS_H_*/\n" >> "vcmdswregisters.h" }

        $1 ~ /^[0-9]+$/ {
                for (i = 1; i <= NF; i++)
                {
                        if ($(i) ~ /^sw_/)
                                fieldCol=i;
                        else if ($(i) ~ /RO/)
                                RW_Col=i;
                        else if ($(i) ~ /RW/)
                                RW_Col=i;
                        else if ($(i) ~ /W1C/)
                                RW_Col=i;
                }
                for (i = 1; i <= fieldCol; i++)
                        rowArray[i]=$(i);
                tmpValue=$(fieldCol+1);
                for (i = fieldCol+2; i < RW_Col; i++)
                        tmpValue=tmpValue$(i);
                rowArray[fieldCol+1]=tmpValue;
                j = fieldCol + 2
                for (i = RW_Col; i <= NF; i++) {
                        rowArray[j]=$(i);
                        j++;
                }

                regIndex=int(rowArray[1]);
                lsb=rowArray[3];
                reg=rowArray[4];
                modelreg=rowArray[4];
                description=rowArray[5];
                access_type=rowArray[6];
                trace_value=0

                msb=lsb
                # 15:10 -> 10 etc
                sub(/[0-9]*:/,"",lsb);
                # 15:10 -> 15 etc
                sub(/:[0-9]*/,"",msb);
                width=msb-lsb+1;
                #printf "---lsb=%d,msb=%d,width=%d-----\n",lsb,msb,width;
                sub(/^sw_/,"hwif_vcmd_",modelreg);
                #printf "---modelreg=%s-----\n",modelreg;

                bit_occupy_high=0;
                bit_occupy_low=0;
                for (k = 1; k <=int(width); k++) {
                        #printf "k=%d\n",k;
                        bit_occupy_high=bit_occupy_high*2;
                        if(bit_occupy_low>=32768)
                                bit_occupy_high=bit_occupy_high+1;
                        bit_occupy_low=bit_occupy_low*2;
                        if(bit_occupy_low>=65536)
                                bit_occupy_low=bit_occupy_low-65536;
                        bit_occupy_low=bit_occupy_low+1;
                }
                #printf "1bit_occupy_high=0x%0x\n",bit_occupy_high;
                #printf "1bit_occupy_low=0x%0x\n",bit_occupy_low;
                for (j = 1; j <= int(lsb); j++) {
                        bit_occupy_high=bit_occupy_high*2;
                        if (bit_occupy_low>=32768)
                                bit_occupy_high=bit_occupy_high+1;
                        bit_occupy_low=bit_occupy_low*2;
                        if(bit_occupy_low>=65536)
                                bit_occupy_low=bit_occupy_low-65536;
                }
                #printf "2bit_occupy_high=0x%0x\n",bit_occupy_high;
                #printf "2bit_occupy_low=0x%0x\n",bit_occupy_low;

                if (int(msb) > 31)
                        printf "%s definition over register limits: msb %d, lsb %d, width %d \n",reg, msb,lsb,width;
                if (regIndex < '$regNum' && reg != "RSV") {
                        ##structure
                        printf "  uai%d %s;\n", width, reg >> "vcmdswregisters.h";
                        ##Directives
                        printf "directive set /$block/SwRegister.%s:rsc -MAP_TO_MODULE {[DirectInput]}\n",
                            reg >> "vcmdswregisters_directives.tcl";
                        ##HW stuff
                        if (width == 1) {
                                printf " '$catapultPrefix'%s'$catapultPostfix'  <= swreg%d(%d);\n",
                                        reg, regIndex, msb >> "hwdriver.vhd";
                                printf " signal '$catapultPrefix'%s'$catapultPostfix' : std_logic;\n",
                                        reg >> "signals.vhd";
                        } else {
                                printf " '$catapultPrefix'%s'$catapultPostfix'  <= swreg%d(%d downto %d);\n",
                                        reg, regIndex, msb, lsb >> "hwdriver.vhd";
                                printf " signal '$catapultPrefix'%s'$catapultPostfix' : std_logic_vector(%d downto 0);\n",
                                        reg, width-1 >> "signals.vhd";
                        }

                        ##System model table
                        # change widths of base addresses to 32 bits
                        regName=toupper(modelreg)",";
                        if (width == 30 && lsb == 2 && reg ~ /base$/) {
                            printf "    VCMDREG(%-40s %3d, 0x%04x%04x, %8d, %8d,%2s,\"%-1s\"),\n",
                                   regName, regIndex*4, bit_occupy_high,bit_occupy_low, 0,trace_value,access_type,description >> "vcmdregistertable.h";
                        } else {
                            printf "    VCMDREG(%-40s %3d, 0x%04x%04x, %8d, %8d, %2s, \"%-1s\"),\n",
                                   regName, regIndex*4,bit_occupy_high,bit_occupy_low, lsb,trace_value,access_type,description >> "vcmdregistertable.h";
                        }
                        ##System model enumerations
                        printf "    %s,\n", toupper(modelreg) >> "vcmdregisterenum.h";
                }
        }
# ' "$fileTmp"


if [ -e $fileTmp ]; then
        rm $fileTmp
fi

#cp vcmdregisterenum.h ../
#cp vcmdregistertable.h ../
