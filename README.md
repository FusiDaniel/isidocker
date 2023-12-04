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