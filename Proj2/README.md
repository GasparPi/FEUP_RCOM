# Configurações

## Configuração dos Tuxs

[Scripts](https://github.com/Gaspar99/FEUP_RCOM/tree/master/Proj2/scripts) para ativar e configurar ethernet ports e ativar routing entre vlans

### Comandos uteis

```bash
# comunicar com a subrede 50 atraves da gateway .253 da subrede 51
route add -net 192.168.50.0/24 gw 192.168.51.253
# apagar a rota para a subrede 50 atraves da gateway .253 da subrede 51
route del -net 192.168.50.0/24 gw 192.168.51.253
```

## Configuração do Switch

1. Executar reset do switch/router
2. Executar sequência de comandos para configurar switch/router
3. Copiar configurações para um ficheiro
```bash
copy running-config flash:mieic07_piri
```
4. Repetir o passo 1 e executar o comando abaixo sempre que quisermos repor as configurações guardadas no ficheiro
```bash
copy flash:mieic07_piri startup-config
reload
```

### Configuração Física

 - **Tux54**
    - S0 - switch console
    - E0 - switch **port 3** (vlan 50)
    - E1 - switch **port 6** (vlan 51)
 - **Tux51**
    - E0 - switch **port 5** (vlan 50)
 - **Tux52**
    - E0 - switch **port 4** (vlan 51)
 - **Router**
    - GE0/1 - switch **port 7** (vlan 51)

### Reset do switch

```bash
# nao esquecer de correr comando 'enable' para privilegios root 
configure terminal
no vlan 2-4096
exit
copy flash:tux5-clean startup-config # para tux y -> tuxy-clean
reload
```

### Criar uma VLAN

```bash
configure terminal
vlan 50 # ou 51
end
```

### Apagar uma VLAN

```bash
configure terminal
no vlan 50 # ou 51
end
```

### Adicionar uma porta X a uma VLAN Y

```bash
configure terminal
interface fastethernet 0/X # porta 1 -> 0/1
switchport mode access
switchport access vlan Y
end
```

### Comandos uteis

```bash
# mostra a configuracao da vlan com id 50
show vlan id 50
# mostra a configuração das vlans (incluindo portas)
show vlan brief
# mostra a configuração das portas (incluindo vlans)
show interface status
# mostra a configuracao da porta 1
show running-config interface fastethernet 0/1
show interfaces fastethernet 0/1 switchport
```

## Configuração do Router

**Porta 0/1 ligada à rede do laboratório 172.16.1.59 (netmask 255.255.255.0)**

```bash
configure terminal
interface gigabitethernet 0/0
ip address 192.168.51.254 255.255.255.0
no shutdown
exit
```

**Adicionar rota para a subrede 50 usando a porta .253 da subrede 51**

```bash
configure terminal
# connect to internet?
ip route 0.0.0.0 0.0.0.0 172.16.1.254
# route para a sub-rede 50 tem que passar pela gateway .253 da subrede 51
ip route 192.168.50.0 255.255.255.0 192.168.51.253
end
```

### Commandos uteis

```bash
# mostra a configuracao da porta ethernet 0
show interface gigabitethernet 0/0
show ip route
```
