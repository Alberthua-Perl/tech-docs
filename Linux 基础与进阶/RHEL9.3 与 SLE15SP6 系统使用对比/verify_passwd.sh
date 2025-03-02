#!/bin/bash

echo -e "\n***** VERIFY PASSWORD CONSISTENCY *****"
##输入待验证的用户
read -p "Please type user name: " USER
##输入待验证的明文密码
##read 命令的 -s 选项将隐藏输入的明文密码
read -s -p "Please type user password: " PASSWORD

#提取用户的密码字段（已设置密码）
SHADOW=$(grep ${USER} /etc/shadow | cut -d ':' -f 2)
echo -e "\nShadow of ${USER}: ${SHADOW}"
#提取用户密码中的 salt 值
SALT=$(grep ${USER} /etc/shadow | cut -d ':' -f 2 | cut -d '$' -f 3)
echo "Salt of previous shadow: ${SALT}"

#使用明文密码与 salt 值生成密码（预设置密码）
CRYPT=$(perl -e "print crypt("${PASSWORD}", q(\$6\$$SALT))")
echo "Crypted password is: ${CRYPT}"

#比较已设置密码与预设置密码的一致性
[[ $SHADOW == $CRYPT ]] && echo "Setup password correctly!"
