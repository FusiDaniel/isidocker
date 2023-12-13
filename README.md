# isidocker

#commands

```
gcc isidocker.c -o "/home/opc/isidocker.e"
sudo systemctl status isidocker-javacontainer

sudo "/home/opc/"isidocker.e shell javacontainer

log everything
journalctl _SYSTEMD_INVOCATION_ID=`systemctl show -p InvocationID --value isidocker-javacontainer.service` -f

log container initialization
journalctl _PID=`systemctl show -p MainPID --value isidocker-javacontainer.service`

log springboot
journalctl _PID=$(pgrep -P $(systemctl show -p MainPID --value isidocker-javacontainer.service))

bash -c '/jdk-20.0.2/bin/java -jar hello-0.0.1-SNAPSHOT.jar' &
ps -ef
mount | grep /proc
mount -t proc proc /proc
mount -t proc proc /home/opc/.isidocker_images/javacontainer/proc
sudo mount -t proc proc /home/opc/.isidocker_images/javacontainer/proc
sudo umount /home/opc/.isidocker_images/javacontainer/proc
netstat -tulpn | grep LISTEN
sudo nsenter --target 3315 --mount --uts --ipc --net --pid -- /bin/bash -c "ps -ef"
sudo nsenter --target 3315 --mount --uts --ipc --net --pid -- /bin/bash -c "ps -ef"
sudo unshare --pid --net --fork --uts --ipc chroot /home/opc/.isidocker_images/javacontainer
sudo unshare --pid --fork chroot /home/opc/.isidocker_images/javacontainer
pstree -p

sudo unshare --pid --net --fork --uts --ipc --mount-proc=/home/opc/.isidocker_images/javacontainer/proc chroot /home/opc/.isidocker_images/javacontainer
lsns -l
lsns -t pid
sudo unshare -p -f --mount-proc
```

```
[Unit]
Description=Isidoker container

[Service]
ExecStart=/home/opc/isidocker.e run javacontainer

[Install]
WantedBy=multi-user.target

Rascunho temporario
```