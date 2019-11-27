# Configurações

## Configuração dos Tuxs

[Scripts](https://github.com/Gaspar99/FEUP_RCOM/tree/master/Proj2/scripts) para ativar e configurar ethernet ports e ativar routing entre vlans

### Comandos uteis

```bash
# comunicar com a subrede 50 atraves da gateway .253 da subrede 51
route add -net 192.168.50.0/24 gw 192.168.51.253
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

### Reset do switch

```bash
# nao esquecer de correr comando 'enable' para privilegios root 
configure terminal
no vlan 2-4096
exit
copy flash:tux5-clean startup-config # para tux y -> tuxy-clean
reload
```

## Comandos uteis

```bash
# mostra a configuração das vlans (incluindo portas)
show vlan brief
# mostra a configuração das portas (incluindo vlans)
show interface status
```
