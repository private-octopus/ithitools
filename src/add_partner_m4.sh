# !/bin/sh
echo "adding partner $1 for collecting M4 data"
echo "this script shall run as superuser, such as:"
echo "    sudo add_partner_m4.sh <login_name>"
echo "Calling adduser:"
adduser $1
echo "Copying the SSH key"
mkdir /home/$1/.ssh
nano /home/$1/.ssh/authorized_keys
chown $1 /home/$1/.ssh/authorized_keys
chgrp $1 /home/$1/.ssh/authorized_keys
chmod go-rwx /home/$1/.ssh/authorized_keys
chown $1 /home/$1/.ssh
chgrp $1 /home/$1/.ssh
chmod go-rwx /home/$1/.ssh
echo "Created the ssh data for $1"
ls -ld /home/$1/.ssh
ls -l /home/$1/.ssh
mkdir /home/$1/data
chown $1 /home/$1/data
chgrp $1 /home/$1/data
chmod go-wx /home/$1/data
echo "Created the folder data for $1"
ls -ld /home/$1/data
echo $1 >>/home/ubuntu/data/m4list
echo "Added $1 to list of M4 data sources"
cat /home/$1/data

