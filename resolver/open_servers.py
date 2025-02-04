import ipaddress
import sys
import traceback

as_table = [
    ["AS13335","cloudflare"],
    ["AS204136","opennic"],
    ["AS12008","neustar"],
    ["AS15169","googlepdns"],
    ["AS4812","dnspai"],
    ["AS23274","dnspai"],
    ["AS132203","dnspod"],
    ["AS131400","dnswatch"],
    ["AS31400","dnswatch"],
    ["AS33517","dyn"],
    ["AS51453","freedns"],
    ["AS60679","freedomworld"],
    ["AS8551","greenteamdns"],
    ["AS1680","greenteamdns"],
    ["AS4808","onedns"],
    ["AS23724","onedns"],
    ["AS57926","safedns"],
    ["AS131621","twnic"],
    ["AS23393","comodo"],
]
n4_table = [
    [ipaddress.ip_network("8.0.0.0/16"),"level3"],
    [ipaddress.ip_network("23.236.73.15/32"),"cleanbrowsing"],
    [ipaddress.ip_network("23.253.163.53/32"),"alternatedns"],
    [ipaddress.ip_network("31.172.2.139/32"),"quad9"],
    [ipaddress.ip_network("42.83.200.0/24"),"cnnic"],
    [ipaddress.ip_network("45.68.21.5/32"),"quad9"],
    [ipaddress.ip_network("45.76.58.192/32"),"cleanbrowsing"],
    [ipaddress.ip_network("45.76.171.37/32"),"cleanbrowsing"],
    [ipaddress.ip_network("45.77.231.112/32"),"cleanbrowsing"],
    [ipaddress.ip_network("45.221.0.0/22"),"quad9"],
    [ipaddress.ip_network("47.252.81.0/24"),"alidns"],
    [ipaddress.ip_network("47.252.85.0/24"),"alidns"],
    [ipaddress.ip_network("54.153.39.191/32"),"yandex-amazon"],
    [ipaddress.ip_network("58.217.249.0/24"),"114dns"],
    [ipaddress.ip_network("60.215.138.0/24"),"114dns"],
    [ipaddress.ip_network("61.135.186.0/24"),"baidu"],
    [ipaddress.ip_network("63.246.32.64/26"),"quad9"],
    [ipaddress.ip_network("63.246.34.0/26"),"quad9"],
    [ipaddress.ip_network("63.246.34.64/26"),"quad9"],
    [ipaddress.ip_network("63.246.35.192/26"),"quad9"],
    [ipaddress.ip_network("63.246.36.0/26"),"quad9"],
    [ipaddress.ip_network("63.246.36.64/26"),"quad9"],
    [ipaddress.ip_network("63.246.36.128/26"),"quad9"],
    [ipaddress.ip_network("63.246.36.192/26"),"quad9"],
    [ipaddress.ip_network("63.246.37.0/26"),"quad9"],
    [ipaddress.ip_network("63.246.37.64/26"),"quad9"],
    [ipaddress.ip_network("63.246.37.128/26"),"quad9"],
    [ipaddress.ip_network("63.246.37.192/26"),"quad9"],
    [ipaddress.ip_network("63.246.38.64/26"),"quad9"],
    [ipaddress.ip_network("63.246.40.64/26"),"quad9"],
    [ipaddress.ip_network("63.246.41.128/26"),"quad9"],
    [ipaddress.ip_network("63.246.41.128/26"),"quad9"],
    [ipaddress.ip_network("63.246.42.64/26"),"quad9"],
    [ipaddress.ip_network("63.246.43.64/26"),"quad9"],
    [ipaddress.ip_network("63.246.44.0/26"),"quad9"],
    [ipaddress.ip_network("63.246.44.64/26"),"quad9"],
    [ipaddress.ip_network("64.62.128.0/17"),"he"],
    [ipaddress.ip_network("66.96.112.0/26"),"quad9"],
    [ipaddress.ip_network("66.96.112.64/26"),"quad9"],
    [ipaddress.ip_network("66.96.112.128/26"),"quad9"],
    [ipaddress.ip_network("66.96.112.192/26"),"quad9"],
    [ipaddress.ip_network("66.96.113.0/26"),"quad9"],
    [ipaddress.ip_network("66.96.113.64/26"),"quad9"],
    [ipaddress.ip_network("66.96.113.128/26"),"quad9"],
    [ipaddress.ip_network("66.96.113.192/26"),"quad9"],
    [ipaddress.ip_network("66.96.114.0/26"),"quad9"],
    [ipaddress.ip_network("66.96.114.64/26"),"quad9"],
    [ipaddress.ip_network("66.96.114.128/26"),"quad9"],
    [ipaddress.ip_network("66.96.114.192/26"),"quad9"],
    [ipaddress.ip_network("66.96.115.0/26"),"quad9"],
    [ipaddress.ip_network("66.96.115.64/26"),"quad9"],
    [ipaddress.ip_network("66.96.115.192/26"),"quad9"],
    [ipaddress.ip_network("66.96.116.0/26"),"quad9"],
    [ipaddress.ip_network("66.96.116.64/26"),"quad9"],
    [ipaddress.ip_network("66.96.116.128/26"),"quad9"],
    [ipaddress.ip_network("66.96.116.192/26"),"quad9"],
    [ipaddress.ip_network("66.96.117.0/26"),"quad9"],
    [ipaddress.ip_network("66.96.117.64/26"),"quad9"],
    [ipaddress.ip_network("66.96.117.128/26"),"quad9"],
    [ipaddress.ip_network("66.96.117.192/26"),"quad9"],
    [ipaddress.ip_network("66.96.118.0/26"),"quad9"],
    [ipaddress.ip_network("66.96.118.64/26"),"quad9"],
    [ipaddress.ip_network("66.96.118.128/26"),"quad9"],
    [ipaddress.ip_network("66.96.118.192/26"),"quad9"],
    [ipaddress.ip_network("66.96.119.0/26"),"quad9"],
    [ipaddress.ip_network("66.96.119.64/26"),"quad9"],
    [ipaddress.ip_network("66.96.119.128/26"),"quad9"],
    [ipaddress.ip_network("66.96.119.192/26"),"quad9"],
    [ipaddress.ip_network("66.96.120.0/26"),"quad9"],
    [ipaddress.ip_network("66.96.120.64/26"),"quad9"],
    [ipaddress.ip_network("66.96.120.128/26"),"quad9"],
    [ipaddress.ip_network("66.96.120.192/26"),"quad9"],
    [ipaddress.ip_network("66.96.121.0/26"),"quad9"],
    [ipaddress.ip_network("66.96.121.64/26"),"quad9"],
    [ipaddress.ip_network("66.96.121.128/26"),"quad9"],
    [ipaddress.ip_network("66.96.121.192/26"),"quad9"],
    [ipaddress.ip_network("66.96.122.0/26"),"quad9"],
    [ipaddress.ip_network("66.96.122.64/26"),"quad9"],
    [ipaddress.ip_network("66.96.122.128/26"),"quad9"],
    [ipaddress.ip_network("66.96.122.192/26"),"quad9"],
    [ipaddress.ip_network("66.96.123.0/26"),"quad9"],
    [ipaddress.ip_network("66.96.123.64/26"),"quad9"],
    [ipaddress.ip_network("66.96.123.128/26"),"quad9"],
    [ipaddress.ip_network("66.96.123.192/26"),"quad9"],
    [ipaddress.ip_network("66.96.124.0/26"),"quad9"],
    [ipaddress.ip_network("66.96.124.64/26"),"quad9"],
    [ipaddress.ip_network("66.96.124.128/26"),"quad9"],
    [ipaddress.ip_network("66.96.124.192/26"),"quad9"],
    [ipaddress.ip_network("66.96.125.0/26"),"quad9"],
    [ipaddress.ip_network("66.96.125.64/26"),"quad9"],
    [ipaddress.ip_network("66.96.125.128/26"),"quad9"],
    [ipaddress.ip_network("66.96.125.192/26"),"quad9"],
    [ipaddress.ip_network("66.96.126.0/26"),"quad9"],
    [ipaddress.ip_network("66.96.126.64/26"),"quad9"],
    [ipaddress.ip_network("66.96.126.128/26"),"quad9"],
    [ipaddress.ip_network("66.96.126.192/26"),"quad9"],
    [ipaddress.ip_network("66.96.127.0/26"),"quad9"],
    [ipaddress.ip_network("66.96.127.64/26"),"quad9"],
    [ipaddress.ip_network("66.96.127.128/26"),"quad9"],
    [ipaddress.ip_network("66.96.127.192/26"),"quad9"],
    [ipaddress.ip_network("66.102.32.0/26"),"quad9"],
    [ipaddress.ip_network("66.102.32.64/26"),"quad9"],
    [ipaddress.ip_network("66.102.32.128/26"),"quad9"],
    [ipaddress.ip_network("66.102.32.192/26"),"quad9"],
    [ipaddress.ip_network("66.102.33.0/26"),"quad9"],
    [ipaddress.ip_network("66.102.33.64/26"),"quad9"],
    [ipaddress.ip_network("66.102.33.128/26"),"quad9"],
    [ipaddress.ip_network("66.102.33.192/26"),"quad9"],
    [ipaddress.ip_network("66.102.34.0/26"),"quad9"],
    [ipaddress.ip_network("66.102.34.64/26"),"quad9"],
    [ipaddress.ip_network("66.102.34.128/26"),"quad9"],
    [ipaddress.ip_network("66.102.34.192/26"),"quad9"],
    [ipaddress.ip_network("66.102.35.0/26"),"quad9"],
    [ipaddress.ip_network("66.102.35.64/26"),"quad9"],
    [ipaddress.ip_network("66.102.35.128/26"),"quad9"],
    [ipaddress.ip_network("66.102.35.192/26"),"quad9"],
    [ipaddress.ip_network("66.102.36.0/26"),"quad9"],
    [ipaddress.ip_network("66.102.36.64/26"),"quad9"],
    [ipaddress.ip_network("66.102.36.128/26"),"quad9"],
    [ipaddress.ip_network("66.102.36.192/26"),"quad9"],
    [ipaddress.ip_network("66.102.37.0/26"),"quad9"],
    [ipaddress.ip_network("66.102.37.64/26"),"quad9"],
    [ipaddress.ip_network("66.102.37.192/26"),"quad9"],
    [ipaddress.ip_network("66.102.38.0/26"),"quad9"],
    [ipaddress.ip_network("66.102.38.64/26"),"quad9"],
    [ipaddress.ip_network("66.102.38.128/26"),"quad9"],
    [ipaddress.ip_network("66.102.38.192/26"),"quad9"],
    [ipaddress.ip_network("66.102.39.0/26"),"quad9"],
    [ipaddress.ip_network("66.102.39.64/26"),"quad9"],
    [ipaddress.ip_network("66.102.39.128/26"),"quad9"],
    [ipaddress.ip_network("66.102.39.192/26"),"quad9"],
    [ipaddress.ip_network("66.102.40.0/26"),"quad9"],
    [ipaddress.ip_network("66.102.40.64/26"),"quad9"],
    [ipaddress.ip_network("66.102.40.128/26"),"quad9"],
    [ipaddress.ip_network("66.102.40.192/26"),"quad9"],
    [ipaddress.ip_network("66.102.41.0/26"),"quad9"],
    [ipaddress.ip_network("66.102.41.64/26"),"quad9"],
    [ipaddress.ip_network("66.102.41.128/26"),"quad9"],
    [ipaddress.ip_network("66.102.41.192/26"),"quad9"],
    [ipaddress.ip_network("66.102.42.0/26"),"quad9"],
    [ipaddress.ip_network("66.102.42.64/26"),"quad9"],
    [ipaddress.ip_network("66.102.46.64/26"),"quad9"],
    [ipaddress.ip_network("66.102.47.64/26"),"quad9"],
    [ipaddress.ip_network("66.102.47.128/26"),"quad9"],
    [ipaddress.ip_network("66.154.113.154/32"),"cleanbrowsing"],
    [ipaddress.ip_network("66.185.112.0/24"),"quad9"],
    [ipaddress.ip_network("66.185.113.0/24"),"quad9"],
    [ipaddress.ip_network("66.185.114.0/24"),"quad9"],
    [ipaddress.ip_network("66.185.114.240/28"),"quad9"],
    [ipaddress.ip_network("66.185.115.0/24"),"quad9"],
    [ipaddress.ip_network("66.185.116.0/24"),"quad9"],
    [ipaddress.ip_network("66.185.117.0/24"),"quad9"],
    [ipaddress.ip_network("66.185.118.0/24"),"quad9"],
    [ipaddress.ip_network("66.185.119.0/24"),"quad9"],
    [ipaddress.ip_network("66.185.120.0/24"),"quad9"],
    [ipaddress.ip_network("66.185.121.0/24"),"quad9"],
    [ipaddress.ip_network("66.185.123.224/27"),"quad9"],
    [ipaddress.ip_network("66.220.0.0/19"),"he"],
    [ipaddress.ip_network("67.215.80.0/24"),"opendns"],
    [ipaddress.ip_network("67.215.82.0/24"),"opendns"],
    [ipaddress.ip_network("67.215.83.0/24"),"opendns"],
    [ipaddress.ip_network("67.215.84.0/24"),"opendns"],
    [ipaddress.ip_network("67.215.85.0/24"),"opendns"],
    [ipaddress.ip_network("67.215.86.0/24"),"opendns"],
    [ipaddress.ip_network("72.11.132.178/32"),"cleanbrowsing"],
    [ipaddress.ip_network("72.52.64.0/18"),"he"],
    [ipaddress.ip_network("74.63.16.0/24"),"quad9"],
    [ipaddress.ip_network("74.63.17.0/24"),"quad9"],
    [ipaddress.ip_network("74.63.18.0/24"),"quad9"],
    [ipaddress.ip_network("74.63.18.240/28"),"quad9"],
    [ipaddress.ip_network("74.63.19.0/24"),"quad9"],
    [ipaddress.ip_network("74.63.19.224/27"),"quad9"],
    [ipaddress.ip_network("74.63.20.0/24"),"quad9"],
    [ipaddress.ip_network("74.63.21.0/24"),"quad9"],
    [ipaddress.ip_network("74.63.22.0/24"),"quad9"],
    [ipaddress.ip_network("74.63.22.224/27"),"quad9"],
    [ipaddress.ip_network("74.63.23.0/24"),"quad9"],
    [ipaddress.ip_network("74.63.24.0/24"),"quad9"],
    [ipaddress.ip_network("74.63.25.0/24"),"quad9"],
    [ipaddress.ip_network("74.63.26.0/24"),"quad9"],
    [ipaddress.ip_network("74.63.27.0/24"),"quad9"],
    [ipaddress.ip_network("74.63.28.0/24"),"quad9"],
    [ipaddress.ip_network("74.63.29.0/24"),"quad9"],
    [ipaddress.ip_network("74.63.30.0/24"),"quad9"],
    [ipaddress.ip_network("74.63.31.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.66.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.68.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.70.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.71.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.72.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.73.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.74.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.75.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.76.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.77.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.77.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.78.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.79.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.80.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.81.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.82.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.83.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.84.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.85.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.86.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.87.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.88.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.89.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.91.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.92.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.93.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.94.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.95.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.98.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.99.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.100.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.101.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.102.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.103.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.104.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.105.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.106.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.107.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.108.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.109.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.110.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.111.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.112.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.113.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.114.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.115.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.116.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.117.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.118.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.119.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.120.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.121.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.122.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.123.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.124.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.125.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.126.0/24"),"quad9"],
    [ipaddress.ip_network("74.80.127.0/24"),"quad9"],
    [ipaddress.ip_network("74.82.0.0/18"),"he"],
    [ipaddress.ip_network("74.125.18.0/25"),"googlepdns"],
    [ipaddress.ip_network("74.125.18.128/26"),"googlepdns"],
    [ipaddress.ip_network("74.125.18.192/26"),"googlepdns"],
    [ipaddress.ip_network("74.125.19.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.40.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.41.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.42.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.44.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.45.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.46.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.47.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.72.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.73.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.74.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.75.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.76.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.77.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.78.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.79.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.80.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.81.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.92.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.112.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.113.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.115.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.176.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.177.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.178.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.179.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.180.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.181.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.182.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.183.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.184.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.185.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.186.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.187.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.190.0/24"),"googlepdns"],
    [ipaddress.ip_network("74.125.191.0/24"),"googlepdns"],
    [ipaddress.ip_network("77.88.56.0/24"),"yandex"],
    [ipaddress.ip_network("81.19.192.0/20"),"vrsgn"],
    [ipaddress.ip_network("83.223.39.0/24"),"freenom"],
    [ipaddress.ip_network("83.223.44.0/24"),"freenom"],
    [ipaddress.ip_network("83.223.45.0/24"),"freenom"],
    [ipaddress.ip_network("83.223.50.0/24"),"freenom"],
    [ipaddress.ip_network("85.255.211.0/24"),"freenom"],
    [ipaddress.ip_network("87.255.36.0/24"),"freenom"],
    [ipaddress.ip_network("89.233.43.71/32"),"uncensoreddns"],
    [ipaddress.ip_network("91.201.65.18/32"),"cleanbrowsing"],
    [ipaddress.ip_network("95.179.130.14/32"),"cleanbrowsing"],
    [ipaddress.ip_network("103.87.108.0/22"),"vrsgn"],
    [ipaddress.ip_network("103.107.199.240/28"),"quad9"],
    [ipaddress.ip_network("103.137.15.236/30"),"quad9"],
    [ipaddress.ip_network("103.137.15.240/28"),"quad9"],
    [ipaddress.ip_network("103.200.96.215/32"),"cleanbrowsing"],
    [ipaddress.ip_network("103.200.218.0/24"),"freenom"],
    [ipaddress.ip_network("103.205.140.168/32"),"cleanbrowsing"],
    [ipaddress.ip_network("104.223.91.42/32"),"cleanbrowsing"],
    [ipaddress.ip_network("109.69.8.51/32"),"puntcat"],
    [ipaddress.ip_network("116.90.72.128/31"),"quad9"],
    [ipaddress.ip_network("116.90.72.224/28"),"quad9"],
    [ipaddress.ip_network("121.40.12.0/24"),"cnnic"],
    [ipaddress.ip_network("130.225.244.166/32"),"uncensoreddns"],
    [ipaddress.ip_network("130.226.161.34/32"),"uncensoreddns"],
    [ipaddress.ip_network("139.180.222.255/32"),"cleanbrowsing"],
    [ipaddress.ip_network("140.82.14.217/32"),"cleanbrowsing"],
    [ipaddress.ip_network("140.82.30.53/32"),"cleanbrowsing"],
    [ipaddress.ip_network("140.82.39.20/32"),"cleanbrowsing"],
    [ipaddress.ip_network("141.98.90.234/32"),"cleanbrowsing"],
    [ipaddress.ip_network("144.202.108.20/32"),"cleanbrowsing"],
    [ipaddress.ip_network("146.112.128.0/21"),"opendns"],
    [ipaddress.ip_network("146.112.136.0/23"),"opendns"],
    [ipaddress.ip_network("146.112.138.0/24"),"opendns"],
    [ipaddress.ip_network("149.28.187.17/32"),"cleanbrowsing"],
    [ipaddress.ip_network("154.16.135.216/32"),"cleanbrowsing"],
    [ipaddress.ip_network("158.38.58.187/32"),"quad9"],
    [ipaddress.ip_network("166.70.78.3/32"),"quad9"],
    [ipaddress.ip_network("167.179.94.245/32"),"cleanbrowsing"],
    [ipaddress.ip_network("170.81.42.58/32"),"cleanbrowsing"],
    [ipaddress.ip_network("172.217.32.0/25"),"googlepdns"],
    [ipaddress.ip_network("172.217.32.128/25"),"googlepdns"],
    [ipaddress.ip_network("172.217.33.0/25"),"googlepdns"],
    [ipaddress.ip_network("172.217.33.128/25"),"googlepdns"],
    [ipaddress.ip_network("172.217.34.0/26"),"googlepdns"],
    [ipaddress.ip_network("172.217.34.64/26"),"googlepdns"],
    [ipaddress.ip_network("172.217.34.128/25"),"googlepdns"],
    [ipaddress.ip_network("172.217.35.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.217.36.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.217.37.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.217.38.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.217.39.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.217.40.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.217.41.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.217.42.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.217.43.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.217.44.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.217.45.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.217.46.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.217.47.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.0.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.1.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.2.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.3.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.4.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.5.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.6.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.7.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.8.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.9.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.10.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.11.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.12.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.13.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.14.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.15.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.192.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.193.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.194.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.195.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.196.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.197.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.198.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.199.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.200.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.201.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.202.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.204.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.205.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.206.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.209.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.210.0/24"),"googlepdns"],
    [ipaddress.ip_network("172.253.211.0/24"),"googlepdns"],
    [ipaddress.ip_network("173.194.90.0/24"),"googlepdns"],
    [ipaddress.ip_network("173.194.91.0/24"),"googlepdns"],
    [ipaddress.ip_network("173.194.93.0/24"),"googlepdns"],
    [ipaddress.ip_network("173.194.94.0/24"),"googlepdns"],
    [ipaddress.ip_network("173.194.95.0/24"),"googlepdns"],
    [ipaddress.ip_network("173.194.96.0/24"),"googlepdns"],
    [ipaddress.ip_network("173.194.97.0/24"),"googlepdns"],
    [ipaddress.ip_network("173.194.98.0/24"),"googlepdns"],
    [ipaddress.ip_network("173.194.99.0/24"),"googlepdns"],
    [ipaddress.ip_network("173.194.100.0/24"),"googlepdns"],
    [ipaddress.ip_network("173.194.101.0/24"),"googlepdns"],
    [ipaddress.ip_network("173.194.102.0/24"),"googlepdns"],
    [ipaddress.ip_network("173.194.103.0/24"),"googlepdns"],
    [ipaddress.ip_network("173.194.168.0/25"),"googlepdns"],
    [ipaddress.ip_network("173.194.168.128/26"),"googlepdns"],
    [ipaddress.ip_network("173.194.168.192/26"),"googlepdns"],
    [ipaddress.ip_network("173.194.169.0/24"),"googlepdns"],
    [ipaddress.ip_network("173.194.170.0/24"),"googlepdns"],
    [ipaddress.ip_network("173.194.171.0/24"),"googlepdns"],
    [ipaddress.ip_network("176.56.180.0/24"),"safedns"],
    [ipaddress.ip_network("176.56.185.0/24"),"safedns"],
    [ipaddress.ip_network("177.101.25.155/32"),"quad9"],
    [ipaddress.ip_network("178.132.70.15/32"),"quad9"],
    [ipaddress.ip_network("179.0.200.40/32"),"quad9"],
    [ipaddress.ip_network("180.76.14.0/24"),"baidu"],
    [ipaddress.ip_network("180.149.143.0/24"),"baidu"],
    [ipaddress.ip_network("184.105.250.0/23"),"he"],
    [ipaddress.ip_network("184.105.252.0/22"),"he"],
    [ipaddress.ip_network("184.170.249.0/24"),"freenom"],
    [ipaddress.ip_network("185.33.111.67/32"),"quad9"],
    [ipaddress.ip_network("185.40.106.224/28"),"quad9"],
    [ipaddress.ip_network("185.48.57.0/24"),"safedns"],
    [ipaddress.ip_network("185.60.86.0/23"),"opendns"],
    [ipaddress.ip_network("185.99.133.41/32"),"cleanbrowsing"],
    [ipaddress.ip_network("185.100.3.0/24"),"vrsgn"],
    [ipaddress.ip_network("185.170.209.30/32"),"cleanbrowsing"],
    [ipaddress.ip_network("185.173.25.88/32"),"cleanbrowsing"],
    [ipaddress.ip_network("185.181.61.4/32"),"cleanbrowsing"],
    [ipaddress.ip_network("188.122.68.208/28"),"quad9"],
    [ipaddress.ip_network("192.34.234.0/23"),"vrsgn"],
    [ipaddress.ip_network("192.34.236.0/23"),"vrsgn"],
    [ipaddress.ip_network("192.34.238.0/24"),"vrsgn"],
    [ipaddress.ip_network("192.68.126.0/23"),"vrsgn"],
    [ipaddress.ip_network("192.68.128.0/23"),"vrsgn"],
    [ipaddress.ip_network("192.68.130.0/24"),"vrsgn"],
    [ipaddress.ip_network("192.81.185.0/24"),"vrsgn"],
    [ipaddress.ip_network("192.81.186.0/23"),"vrsgn"],
    [ipaddress.ip_network("192.81.188.0/23"),"vrsgn"],
    [ipaddress.ip_network("192.221.0.0/16"),"level3"],
    [ipaddress.ip_network("193.105.196.194/32"),"quad9"],
    [ipaddress.ip_network("194.54.137.35/32"),"quad9"],
    [ipaddress.ip_network("194.116.81.227/32"),"quad9"],
    [ipaddress.ip_network("194.149.135.211/32"),"quad9"],
    [ipaddress.ip_network("196.251.250.197/32"),"cleanbrowsing"],
    [ipaddress.ip_network("197.230.227.227/32"),"quad9"],
    [ipaddress.ip_network("198.11.178.0/23"),"alidns"],
    [ipaddress.ip_network("198.101.242.82/32"),"alternatidns"],
    [ipaddress.ip_network("199.7.48.0/20"),"vrsgn"],
    [ipaddress.ip_network("199.247.15.163/32"),"cleanbrowsing"],
    [ipaddress.ip_network("200.12.251.67/32"),"quad9"],
    [ipaddress.ip_network("200.25.1.192/28"),"quad9"],
    [ipaddress.ip_network("200.25.22.176/28"),"quad9"],
    [ipaddress.ip_network("200.25.53.0/26"),"quad9"],
    [ipaddress.ip_network("200.25.56.96/28"),"quad9"],
    [ipaddress.ip_network("200.25.57.0/26"),"quad9"],
    [ipaddress.ip_network("202.52.0.6/32"),"quad9"],
    [ipaddress.ip_network("203.23.178.32/28"),"quad9"],
    [ipaddress.ip_network("203.75.51.0/24"),"quad101"],
    [ipaddress.ip_network("203.119.33.0/24"),"cnnic"],
    [ipaddress.ip_network("203.144.48.0/20"),"vrsgn"],
    [ipaddress.ip_network("204.194.237.0/24"),"opendns"],
    [ipaddress.ip_network("204.194.238.0/24"),"opendns"],
    [ipaddress.ip_network("204.194.239.0/24"),"opendns"],
    [ipaddress.ip_network("206.200.228.0/23"),"quad9"],
    [ipaddress.ip_network("206.200.230.0/23"),"quad9"],
    [ipaddress.ip_network("207.148.11.35/32"),"cleanbrowsing"],
    [ipaddress.ip_network("208.67.216.0/24"),"opendns"],
    [ipaddress.ip_network("208.67.217.0/24"),"opendns"],
    [ipaddress.ip_network("208.67.219.0/24"),"opendns"],
    [ipaddress.ip_network("208.69.32.0/24"),"opendns"],
    [ipaddress.ip_network("208.69.33.0/24"),"opendns"],
    [ipaddress.ip_network("208.69.34.0/24"),"opendns"],
    [ipaddress.ip_network("208.69.35.0/24"),"opendns"],
    [ipaddress.ip_network("208.69.36.0/24"),"opendns"],
    [ipaddress.ip_network("208.69.37.0/24"),"opendns"],
    [ipaddress.ip_network("209.51.161.0/24"),"he"],
    [ipaddress.ip_network("209.131.128.0/18"),"vrsgn"],
    [ipaddress.ip_network("211.144.10.0/24"),"cnnic"],
    [ipaddress.ip_network("213.183.41.238/32"),"cleanbrowsing"],
    [ipaddress.ip_network("213.183.63.23/32"),"cleanbrowsing"],
    [ipaddress.ip_network("213.226.68.68/32"),"cleanbrowsing"],
    [ipaddress.ip_network("216.21.2.0/24"),"quad9"],
    [ipaddress.ip_network("216.21.3.0/24"),"quad9"],
    [ipaddress.ip_network("216.66.0.0/18"),"he"],
    [ipaddress.ip_network("216.66.64.0/19"),"he"],
    [ipaddress.ip_network("216.87.142.0/24"),"vrsgn"],
    [ipaddress.ip_network("216.218.128.0/17"),"he"],
    [ipaddress.ip_network("217.30.80.0/20"),"vrsgn"],
    [ipaddress.ip_network("219.99.143.0/24"),"freenom"],
]
n6_table = [
    [ipaddress.ip_network("2001:470::/48"),"he"],
    [ipaddress.ip_network("2001:500:15:200::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:400::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:600::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:800::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:900::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:1100::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:1300::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:1400::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:1700::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:2100::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:2300::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:2500::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:2900::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:3000::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:3100::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:3200::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:3300::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:3500::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:4400::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:4600::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:4800::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:4900::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:5000::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:5400::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:5500::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:5700::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:5800::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:6000::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:6200::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:6300::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:6400::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:6700::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:6900::/56"),"quad9"],
    [ipaddress.ip_network("2001:500:15:7000::/56"),"quad9"],
    [ipaddress.ip_network("2001:878:0:e000::/64"),"uncensoreddns"],
    [ipaddress.ip_network("2001:878:0:e000::/64"),"uncensoreddns"],
    [ipaddress.ip_network("2001:dc7::/32"),"cnnic"],
    [ipaddress.ip_network("2001:4800:780e::/48"),"alternatedns"],
    [ipaddress.ip_network("2001:4801:7825::/48"),"alternatedns"],
    [ipaddress.ip_network("2401:3bc0:c::2/127"),"quad9"],
    [ipaddress.ip_network("2401:3bc0:c::4/127"),"quad9"],
    [ipaddress.ip_network("2401:3bc0:c:9::/64"),"quad9"],
    [ipaddress.ip_network("2401:3bc0:d:9::/64"),"quad9"],
    [ipaddress.ip_network("2401:3bc0:600:9::/64"),"quad9"],
    [ipaddress.ip_network("2401:3bc0:100f:9::/64"),"quad9"],
    [ipaddress.ip_network("2402:79c0::/32"),"vrsgn"],
    [ipaddress.ip_network("2404:6800:4000::/48"),"googlepdns"],
    [ipaddress.ip_network("2404:6800:4003::/48"),"googlepdns"],
    [ipaddress.ip_network("2404:6800:4005::/48"),"googlepdns"],
    [ipaddress.ip_network("2404:6800:4006::/48"),"googlepdns"],
    [ipaddress.ip_network("2404:6800:4008::/48"),"googlepdns"],
    [ipaddress.ip_network("2404:6800:400a::/48"),"googlepdns"],
    [ipaddress.ip_network("2404:6800:400b::/48"),"googlepdns"],
    [ipaddress.ip_network("2404:6800:4013::/48"),"googlepdns"],
    [ipaddress.ip_network("2406:f400:22::/48"),"freenom"],
    [ipaddress.ip_network("2607:f7a0:1::/48"),"freenom"],
    [ipaddress.ip_network("2607:f8b0:4001::/48"),"googlepdns"],
    [ipaddress.ip_network("2607:f8b0:4002::/48"),"googlepdns"],
    [ipaddress.ip_network("2607:f8b0:4003::/48"),"googlepdns"],
    [ipaddress.ip_network("2607:f8b0:4004::/52"),"googlepdns"],
    [ipaddress.ip_network("2607:f8b0:4004:1000::/52"),"googlepdns"],
    [ipaddress.ip_network("2607:f8b0:400c::/48"),"googlepdns"],
    [ipaddress.ip_network("2607:f8b0:400d::/48"),"googlepdns"],
    [ipaddress.ip_network("2607:f8b0:400e::/48"),"googlepdns"],
    [ipaddress.ip_network("2607:f8b0:4020::/48"),"googlepdns"],
    [ipaddress.ip_network("2607:f8b0:4023::/48"),"googlepdns"],
    [ipaddress.ip_network("2620:0:876:1000::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:1100::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:1200::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:1300::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:1400::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:1500::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:1600::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:1700::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:1800::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:2000::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:2100::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:2200::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:2300::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:2400::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:2500::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:2600::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:2700::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:2800::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:2900::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:3000::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:3100::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:3200::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:3300::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:3400::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:3500::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:3600::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:3700::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:3800::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:3900::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:4000::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:4100::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:4200::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:4300::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:4400::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:4500::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:4600::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:4700::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:4800::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:4900::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:5000::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:5100::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:5200::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:5300::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:5400::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:5500::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:5600::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:5700::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:5800::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:5900::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:6000::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:6100::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:6200::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:6300::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:6400::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:6500::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:6600::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:6700::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:6800::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:6900::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:7000::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:7100::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:7200::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:7300::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:7400::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:7500::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:7600::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:7700::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:7800::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:7900::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:8000::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:8100::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:8200::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:8300::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:8400::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:8500::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:8600::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:8700::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:8800::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:8900::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:9000::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:9100::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:9200::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:9300::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:9400::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:9500::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:9600::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:9700::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:9800::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:876:9900::/56"),"quad9"],
    [ipaddress.ip_network("2620:0:877:2900::/64"),"quad9"],
    [ipaddress.ip_network("2620:0:877:3300::/64"),"quad9"],
    [ipaddress.ip_network("2620:0:877:3400::/64"),"quad9"],
    [ipaddress.ip_network("2620:0:877:4000::/64"),"quad9"],
    [ipaddress.ip_network("2620:0:877:4500::/64"),"quad9"],
    [ipaddress.ip_network("2620:0:877:4600::/64"),"quad9"],
    [ipaddress.ip_network("2620:0:877:4700::/64"),"quad9"],
    [ipaddress.ip_network("2620:0:877:4800::/64"),"quad9"],
    [ipaddress.ip_network("2620:0:877:4900::/64"),"quad9"],
    [ipaddress.ip_network("2620:0:877:5100::/64"),"quad9"],
    [ipaddress.ip_network("2620:0:877:5600::/64"),"quad9"],
    [ipaddress.ip_network("2620:0:877:5600::/64"),"quad9"],
    [ipaddress.ip_network("2620:0:877:6300::/64"),"quad9"],
    [ipaddress.ip_network("2620:0:877:6700::/64"),"quad9"],
    [ipaddress.ip_network("2620:0:877:7600::/64"),"quad9"],
    [ipaddress.ip_network("2620:0:877:8100::/64"),"quad9"],
    [ipaddress.ip_network("2620:0:877:9500::/64"),"quad9"],
    [ipaddress.ip_network("2620:0:cc3::/48"),"opendns"],
    [ipaddress.ip_network("2620:0:cc4::/48"),"opendns"],
    [ipaddress.ip_network("2620:0:cc5::/48"),"opendns"],
    [ipaddress.ip_network("2620:0:cc6::/48"),"opendns"],
    [ipaddress.ip_network("2620:0:cc7::/48"),"opendns"],
    [ipaddress.ip_network("2620:0:cc8::/48"),"opendns"],
    [ipaddress.ip_network("2620:0:cc9::/48"),"opendns"],
    [ipaddress.ip_network("2620:0:cca::/48"),"opendns"],
    [ipaddress.ip_network("2620:0:ccb::/48"),"opendns"],
    [ipaddress.ip_network("2620:0:cce::/48"),"opendns"],
    [ipaddress.ip_network("2620:0:ccf::/48"),"opendns"],
    [ipaddress.ip_network("2620:74::/32"),"vrsgn"],
    [ipaddress.ip_network("2620:119:10::/48"),"opendns"],
    [ipaddress.ip_network("2620:119:11::/48"),"opendns"],
    [ipaddress.ip_network("2620:119:12::/48"),"opendns"],
    [ipaddress.ip_network("2620:119:13::/48"),"opendns"],
    [ipaddress.ip_network("2620:171:1::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:2::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:4::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:6::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:8::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:10::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:11::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:13::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:14::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:17::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:18::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:23::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:24::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:25::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:28::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:30::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:31::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:32::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:33::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:34::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:35::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:36::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:37::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:38::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:42::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:43::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:48::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:49::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:50::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:51::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:52::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:53::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:54::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:55::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:56::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:57::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:58::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:61::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:64::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:65::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:66::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:66::/64"),"quad9"],
    [ipaddress.ip_network("2620:171:67::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:68::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:71::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:74::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:76::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:c6::/64"),"quad9"],
    [ipaddress.ip_network("2620:171:c7::/64"),"quad9"],
    [ipaddress.ip_network("2620:171:cc::/64"),"quad9"],
    [ipaddress.ip_network("2620:171:ce::/64"),"quad9"],
    [ipaddress.ip_network("2620:171:cf::/64"),"quad9"],
    [ipaddress.ip_network("2620:171:d2::/64"),"quad9"],
    [ipaddress.ip_network("2620:171:d5::/64"),"quad9"],
    [ipaddress.ip_network("2620:171:df::/64"),"quad9"],
    [ipaddress.ip_network("2620:171:e0::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:e1::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:e2::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:e2:f0::/64"),"quad9"],
    [ipaddress.ip_network("2620:171:e3::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:e4::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:e5::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:e6::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:e7::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:e8::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:e9::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:ea::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:eb:f0::/64"),"quad9"],
    [ipaddress.ip_network("2620:171:f0::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:f1::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:f2::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:f2:f0::/64"),"quad9"],
    [ipaddress.ip_network("2620:171:f3::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:f3:f0::/64"),"quad9"],
    [ipaddress.ip_network("2620:171:f4::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:f5::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:f6::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:f6:f0::/64"),"quad9"],
    [ipaddress.ip_network("2620:171:f7::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:f8::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:f9::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:fa::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:fb::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:fc::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:fd::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:fe::/48"),"quad9"],
    [ipaddress.ip_network("2620:171:ff::/48"),"quad9"],
    [ipaddress.ip_network("2800:1e0:a06::/64"),"quad9"],
    [ipaddress.ip_network("2800:1e0:1020:2::/64"),"quad9"],
    [ipaddress.ip_network("2800:1e0:1020:4::/64"),"quad9"],
    [ipaddress.ip_network("2800:1e0:1060:3::/64"),"quad9"],
    [ipaddress.ip_network("2800:1e0:3000:2::/64"),"quad9"],
    [ipaddress.ip_network("2800:3f0:4001::/48"),"googlepdns"],
    [ipaddress.ip_network("2800:3f0:4003::/48"),"googlepdns"],
    [ipaddress.ip_network("2a00:ec8:400::/48"),"freenom"],
    [ipaddress.ip_network("2a00:f80:56::/48"),"freenom"],
    [ipaddress.ip_network("2a00:1450:4001::/48"),"googlepdns"],
    [ipaddress.ip_network("2a00:1450:4009::/48"),"googlepdns"],
    [ipaddress.ip_network("2a00:1450:400a::/48"),"googlepdns"],
    [ipaddress.ip_network("2a00:1450:400b::/48"),"googlepdns"],
    [ipaddress.ip_network("2a00:1450:400c::/48"),"googlepdns"],
    [ipaddress.ip_network("2a00:1450:4010::/48"),"googlepdns"],
    [ipaddress.ip_network("2a00:1450:4013::/48"),"googlepdns"],
    [ipaddress.ip_network("2a00:1450:4025::/48"),"googlepdns"],
    [ipaddress.ip_network("2a00:1508:0:4::/64"),"puntcat"],
    [ipaddress.ip_network("2a00:1637:409::/48"),"quad9"],
    [ipaddress.ip_network("2a01:3a0:53:53::/64"),"uncensoreddns"],
    [ipaddress.ip_network("2a04:e4c0:10::/48"),"opendns"],
    [ipaddress.ip_network("2a04:e4c0:12::/48"),"opendns"],
    [ipaddress.ip_network("2a04:e4c0:14::/48"),"opendns"],
    [ipaddress.ip_network("2a04:e4c0:15::/48"),"opendns"],
    [ipaddress.ip_network("2a04:e4c0:16::/48"),"opendns"],
    [ipaddress.ip_network("2a04:e4c0:17::/48"),"opendns"],
    [ipaddress.ip_network("2a04:e4c0:18::/48"),"opendns"],
    [ipaddress.ip_network("2a04:e4c0:20::/48"),"opendns"],
    [ipaddress.ip_network("2a04:e4c0:21::/48"),"opendns"],
    [ipaddress.ip_network("2a04:e4c0:22::/48"),"opendns"],
    [ipaddress.ip_network("2a04:e4c0:23::/48"),"opendns"],
    [ipaddress.ip_network("2a04:e4c0:24::/48"),"opendns"],
    [ipaddress.ip_network("2a04:e4c0:25::/48"),"opendns"],
    [ipaddress.ip_network("2a04:e4c0:30::/48"),"opendns"],
    [ipaddress.ip_network("2a04:e4c0:31::/48"),"opendns"],
    [ipaddress.ip_network("2a04:e4c0:40::/48"),"opendns"],
    [ipaddress.ip_network("2a0c:e080:0:3::/64"),"quad9"],
]

open_server_nets = [
    [ "66.102.34.192/26", "quad9"], \
    [ "66.102.35.192/26", "quad9"], \
    [ "74.63.25.0/24", "quad9"], \
    [ "74.80.109.0/24", "quad9"], \
    [ "66.96.126.0/26", "quad9"], \
    [ "66.96.120.128/26", "quad9"], \
    [ "74.63.17.0/24", "quad9"], \
    [ "74.80.112.0/24", "quad9"], \
    [ "66.102.32.128/26", "quad9"], \
    [ "66.185.113.0/24", "quad9"], \
    [ "74.80.120.0/24", "quad9"], \
    [ "66.102.40.192/26", "quad9"], \
    [ "74.80.125.0/24", "quad9"], \
    [ "66.102.37.64/26", "quad9"], \
    [ "66.96.121.0/26", "quad9"], \
    [ "66.102.40.64/26", "quad9"], \
    [ "66.96.122.192/26", "quad9"], \
    [ "66.96.125.192/26", "quad9"], \
    [ "74.80.89.0/24", "quad9"], \
    [ "66.96.113.192/26", "quad9"], \
    [ "74.80.119.0/24", "quad9"], \
    [ "66.102.41.192/26", "quad9"], \
    [ "66.96.114.64/26", "quad9"], \
    [ "66.96.114.0/26", "quad9"], \
    [ "66.102.34.0/26", "quad9"], \
    [ "74.63.18.0/24", "quad9"], \
    [ "74.80.80.0/24", "quad9"], \
    [ "66.102.38.192/26", "quad9"], \
    [ "74.63.22.0/24", "quad9"], \
    [ "74.80.117.0/24", "quad9"], \
    [ "66.96.113.0/26", "quad9"], \
    [ "74.80.85.0/24", "quad9"], \
    [ "66.96.125.128/26", "quad9"], \
    [ "74.63.30.0/24", "quad9"], \
    [ "74.80.93.0/24", "quad9"], \
    [ "66.96.127.192/26", "quad9"], \
    [ "66.96.119.0/26", "quad9"], \
    [ "66.96.122.64/26", "quad9"], \
    [ "66.102.42.64/26", "quad9"], \
    [ "66.185.112.0/24", "quad9"], \
    [ "74.80.110.0/24", "quad9"], \
    [ "66.96.120.64/26", "quad9"], \
    [ "66.102.39.128/26", "quad9"], \
    [ "74.80.104.0/24", "quad9"], \
    [ "66.96.126.64/26", "quad9"], \
    [ "74.80.83.0/24", "quad9"], \
    [ "66.185.121.0/24", "quad9"], \
    [ "74.80.92.0/24", "quad9"], \
    [ "66.96.112.128/26", "quad9"], \
    [ "66.96.126.192/26", "quad9"], \
    [ "66.96.118.64/26", "quad9"], \
    [ "206.200.228.0/23", "quad9"], \
    [ "74.63.24.0/24", "quad9"], \
    [ "74.80.98.0/24", "quad9"], \
    [ "66.96.127.64/26", "quad9"], \
    [ "66.96.116.128/26", "quad9"], \
    [ "66.96.112.64/26", "quad9"], \
    [ "66.102.36.192/26", "quad9"], \
    [ "66.96.125.0/26", "quad9"], \
    [ "66.102.40.0/26", "quad9"], \
    [ "74.63.27.0/24", "quad9"], \
    [ "74.80.115.0/24", "quad9"], \
    [ "66.102.35.64/26", "quad9"], \
    [ "74.80.78.0/24", "quad9"], \
    [ "74.80.74.0/24", "quad9"], \
    [ "66.185.119.0/24", "quad9"], \
    [ "74.80.105.0/24", "quad9"], \
    [ "45.221.0.0/22", "quad9"], \
    [ "74.63.23.0/24", "quad9"], \
    [ "74.80.68.0/24", "quad9"], \
    [ "66.96.125.64/26", "quad9"], \
    [ "74.80.76.0/24", "quad9"], \
    [ "74.80.106.0/24", "quad9"], \
    [ "202.52.0.6/32", "quad9"], \
    [ "66.96.119.64/26", "quad9"], \
    [ "66.96.119.128/26", "quad9"], \
    [ "66.96.122.128/26", "quad9"], \
    [ "66.96.117.192/26", "quad9"], \
    [ "66.102.35.0/26", "quad9"], \
    [ "66.96.117.64/26", "quad9"], \
    [ "74.80.94.0/24", "quad9"], \
    [ "66.96.115.64/26", "quad9"], \
    [ "66.96.120.192/26", "quad9"], \
    [ "66.102.35.128/26", "quad9"], \
    [ "66.96.120.0/26", "quad9"], \
    [ "66.102.36.64/26", "quad9"], \
    [ "66.102.36.0/26", "quad9"], \
    [ "74.63.26.0/24", "quad9"], \
    [ "74.80.123.0/24", "quad9"], \
    [ "66.102.39.192/26", "quad9"], \
    [ "66.185.114.0/24", "quad9"], \
    [ "66.185.116.0/24", "quad9"], \
    [ "74.80.113.0/24", "quad9"], \
    [ "74.80.114.0/24", "quad9"], \
    [ "74.80.126.0/24", "quad9"], \
    [ "66.96.123.192/26", "quad9"], \
    [ "66.96.117.0/26", "quad9"], \
    [ "66.102.38.0/26", "quad9"], \
    [ "66.96.119.192/26", "quad9"], \
    [ "74.80.102.0/24", "quad9"], \
    [ "74.80.87.0/24", "quad9"], \
    [ "66.102.38.64/26", "quad9"], \
    [ "66.96.123.128/26", "quad9"], \
    [ "66.96.118.128/26", "quad9"], \
    [ "74.63.28.0/24", "quad9"], \
    [ "74.80.100.0/24", "quad9"], \
    [ "66.96.127.0/26", "quad9"], \
    [ "66.102.38.128/26", "quad9"], \
    [ "66.96.123.0/26", "quad9"], \
    [ "66.96.118.192/26", "quad9"], \
    [ "66.102.40.128/26", "quad9"], \
    [ "66.96.124.64/26", "quad9"], \
    [ "66.102.32.0/26", "quad9"], \
    [ "66.96.113.64/26", "quad9"], \
    [ "66.96.121.64/26", "quad9"], \
    [ "74.80.95.0/24", "quad9"], \
    [ "74.80.108.0/24", "quad9"], \
    [ "74.63.21.0/24", "quad9"], \
    [ "74.80.99.0/24", "quad9"], \
    [ "66.96.116.64/26", "quad9"], \
    [ "74.63.29.0/24", "quad9"], \
    [ "74.80.116.0/24", "quad9"], \
    [ "74.80.66.0/24", "quad9"], \
    [ "66.96.124.128/26", "quad9"], \
    [ "66.102.41.64/26", "quad9"], \
    [ "74.80.121.0/24", "quad9"], \
    [ "66.102.42.0/26", "quad9"], \
    [ "206.200.230.0/23", "quad9"], \
    [ "74.63.16.0/24", "quad9"], \
    [ "74.80.111.0/24", "quad9"], \
    [ "66.185.123.224/27", "quad9"], \
    [ "74.80.107.0/24", "quad9"], \
    [ "66.102.36.128/26", "quad9"], \
    [ "74.80.103.0/24", "quad9"], \
    [ "66.96.116.192/26", "quad9"], \
    [ "66.102.33.0/26", "quad9"], \
    [ "66.102.32.192/26", "quad9"], \
    [ "74.80.70.0/24", "quad9"], \
    [ "66.96.115.0/26", "quad9"], \
    [ "66.96.121.192/26", "quad9"], \
    [ "66.96.122.0/26", "quad9"], \
    [ "74.63.19.224/27", "quad9"], \
    [ "66.102.41.128/26", "quad9"], \
    [ "66.185.120.0/24", "quad9"], \
    [ "74.80.86.0/24", "quad9"], \
    [ "66.96.121.128/26", "quad9"], \
    [ "66.96.124.192/26", "quad9"], \
    [ "66.96.116.0/26", "quad9"], \
    [ "66.96.126.128/26", "quad9"], \
    [ "66.96.112.0/26", "quad9"], \
    [ "66.96.127.128/26", "quad9"], \
    [ "66.96.115.192/26", "quad9"], \
    [ "216.21.2.0/24", "quad9"], \
    [ "216.21.3.0/24", "quad9"], \
    [ "66.96.117.128/26", "quad9"], \
    [ "66.102.32.64/26", "quad9"], \
    [ "74.63.19.0/24", "quad9"], \
    [ "74.80.79.0/24", "quad9"], \
    [ "66.102.34.128/26", "quad9"], \
    [ "66.185.115.0/24", "quad9"], \
    [ "74.80.101.0/24", "quad9"], \
    [ "74.80.75.0/24", "quad9"], \
    [ "66.102.41.0/26", "quad9"], \
    [ "74.63.20.0/24", "quad9"], \
    [ "74.80.122.0/24", "quad9"], \
    [ "74.80.73.0/24", "quad9"], \
    [ "74.80.77.0/24", "quad9"], \
    [ "66.102.33.128/26", "quad9"], \
    [ "66.102.39.0/26", "quad9"], \
    [ "66.96.114.128/26", "quad9"], \
    [ "74.80.82.0/24", "quad9"], \
    [ "66.102.39.64/26", "quad9"], \
    [ "74.63.31.0/24", "quad9"], \
    [ "74.80.127.0/24", "quad9"], \
    [ "74.80.71.0/24", "quad9"], \
    [ "66.102.34.64/26", "quad9"], \
    [ "74.80.72.0/24", "quad9"], \
    [ "66.96.124.0/26", "quad9"], \
    [ "66.102.37.0/26", "quad9"], \
    [ "74.80.118.0/24", "quad9"], \
    [ "66.102.33.64/26", "quad9"], \
    [ "66.96.118.0/26", "quad9"], \
    [ "74.80.84.0/24", "quad9"], \
    [ "194.116.81.227/32", "quad9"], \
    [ "66.185.117.0/24", "quad9"], \
    [ "74.80.91.0/24", "quad9"], \
    [ "66.102.37.192/26", "quad9"], \
    [ "66.185.118.0/24", "quad9"], \
    [ "74.80.81.0/24", "quad9"], \
    [ "74.80.124.0/24", "quad9"], \
    [ "66.96.123.64/26", "quad9"], \
    [ "66.96.113.128/26", "quad9"], \
    [ "66.102.33.192/26", "quad9"], \
    [ "74.80.88.0/24", "quad9"], \
    [ "66.96.114.192/26", "quad9"], \
    [ "185.33.111.67/32", "quad9"], \
    [ "63.246.38.64/26", "quad9"], \
    [ "158.38.58.187/32", "quad9"], \
    [ "63.246.32.64/26", "quad9"], \
    [ "166.70.78.3/32", "quad9"], \
    [ "66.185.114.240/28", "quad9"], \
    [ "63.246.37.0/26", "quad9"], \
    [ "63.246.36.192/26", "quad9"], \
    [ "63.246.34.0/26", "quad9"], \
    [ "63.246.36.0/26", "quad9"], \
    [ "203.23.178.32/28", "quad9"], \
    [ "188.122.68.208/28", "quad9"], \
    [ "63.246.35.192/26", "quad9"], \
    [ "63.246.36.64/26", "quad9"], \
    [ "31.172.2.139/32", "quad9"], \
    [ "63.246.36.128/26", "quad9"], \
    [ "178.132.70.15/32", "quad9"], \
    [ "200.25.57.0/26", "quad9"], \
    [ "74.80.77.0/24", "quad9"], \
    [ "193.105.196.194/32", "quad9"], \
    [ "66.96.112.192/26", "quad9"], \
    [ "103.137.15.240/28", "quad9"], \
    [ "103.137.15.236/30", "quad9"], \
    [ "116.90.72.224/28", "quad9"], \
    [ "116.90.72.128/31", "quad9"], \
    [ "66.102.46.64/26", "quad9"], \
    [ "103.107.199.240/28", "quad9"], \
    [ "63.246.37.64/26", "quad9"], \
    [ "66.102.47.64/26", "quad9"], \
    [ "185.40.106.224/28", "quad9"], \
    [ "200.25.53.0/26", "quad9"], \
    [ "63.246.41.128/26", "quad9"], \
    [ "197.230.227.227/32", "quad9"], \
    [ "66.102.47.128/26", "quad9"], \
    [ "194.54.137.35/32", "quad9"], \
    [ "63.246.40.64/26", "quad9"], \
    [ "194.149.135.211/32", "quad9"], \
    [ "200.25.22.176/28", "quad9"], \
    [ "200.25.1.192/28", "quad9"], \
    [ "200.25.56.96/28", "quad9"], \
    [ "63.246.34.64/26", "quad9"], \
    [ "179.0.200.40/32", "quad9"], \
    [ "63.246.37.192/26", "quad9"], \
    [ "177.101.25.155/32", "quad9"], \
    [ "63.246.44.0/26", "quad9"], \
    [ "63.246.37.128/26", "quad9"], \
    [ "45.68.21.5/32", "quad9"], \
    [ "74.63.22.224/27", "quad9"], \
    [ "63.246.44.64/26", "quad9"], \
    [ "200.12.251.67/32", "quad9"], \
    [ "74.63.18.240/28", "quad9"], \
    [ "63.246.42.64/26", "quad9"], \
    [ "63.246.43.64/26", "quad9"], \
    [ "63.246.41.128/26", "quad9"], \
    [ "2620:0:877:5600:0:0:0:0/64", "quad9"], \
    [ "2620:171:df:0:0:0:0:0/64", "quad9"], \
    [ "2620:0:877:9500:0:0:0:0/64", "quad9"], \
    [ "2620:171:f2:f0:0:0:0:0/64", "quad9"], \
    [ "2620:0:877:8100:0:0:0:0/64", "quad9"], \
    [ "2620:171:f6:f0:0:0:0:0/64", "quad9"], \
    [ "2620:0:877:7600:0:0:0:0/64", "quad9"], \
    [ "2620:171:c7:0:0:0:0:0/64", "quad9"], \
    [ "2620:171:c6:0:0:0:0:0/64", "quad9"], \
    [ "2620:171:cc:0:0:0:0:0/64", "quad9"], \
    [ "2800:1e0:1020:4:0:0:0:0/64", "quad9"], \
    [ "2800:1e0:a06:0:0:0:0:0/64", "quad9"], \
    [ "2800:1e0:1060:3:0:0:0:0/64", "quad9"], \
    [ "2620:0:877:6700:0:0:0:0/64", "quad9"], \
    [ "2620:0:877:3400:0:0:0:0/64", "quad9"], \
    [ "2620:0:877:5600:0:0:0:0/64", "quad9"], \
    [ "2401:3bc0:600:9:0:0:0:0/64", "quad9"], \
    [ "2620:0:877:6300:0:0:0:0/64", "quad9"], \
    [ "2620:0:877:3300:0:0:0:0/64", "quad9"], \
    [ "2800:1e0:3000:2:0:0:0:0/64", "quad9"], \
    [ "2a0c:e080:0:3:0:0:0:0/64", "quad9"], \
    [ "2620:0:877:2900:0:0:0:0/64", "quad9"], \
    [ "2401:3bc0:d:9:0:0:0:0/64", "quad9"], \
    [ "2401:3bc0:c:9:0:0:0:0/64", "quad9"], \
    [ "2401:3bc0:c::2/127", "quad9"], \
    [ "2401:3bc0:c::4/127", "quad9"], \
    [ "2620:171:ce:0:0:0:0:0/64", "quad9"], \
    [ "2620:171:66:0:0:0:0:0/64", "quad9"], \
    [ "2800:1e0:1020:2:0:0:0:0/64", "quad9"], \
    [ "2620:0:877:4700:0:0:0:0/64", "quad9"], \
    [ "2620:0:877:4600:0:0:0:0/64", "quad9"], \
    [ "2620:171:d2:0:0:0:0:0/64", "quad9"], \
    [ "2a00:1637:409:0:0:0:0:0/48", "quad9"], \
    [ "2401:3bc0:100f:9:0:0:0:0/64", "quad9"], \
    [ "2620:0:877:4500:0:0:0:0/64", "quad9"], \
    [ "2620:0:877:4000:0:0:0:0/64", "quad9"], \
    [ "2620:0:877:4800:0:0:0:0/64", "quad9"], \
    [ "2620:0:877:5100:0:0:0:0/64", "quad9"], \
    [ "2620:171:e2:f0:0:0:0:0/64", "quad9"], \
    [ "2620:171:cf:0:0:0:0:0/64", "quad9"], \
    [ "2620:171:d5:0:0:0:0:0/64", "quad9"], \
    [ "2620:0:877:4900:0:0:0:0/64", "quad9"], \
    [ "2001:0500:0015:0200::/56", "quad9"], \
    [ "2001:0500:0015:0400::/56", "quad9"], \
    [ "2001:0500:0015:0600::/56", "quad9"], \
    [ "2001:0500:0015:0800::/56", "quad9"], \
    [ "2001:0500:0015:0900::/56", "quad9"], \
    [ "2001:0500:0015:1100::/56", "quad9"], \
    [ "2001:0500:0015:1300::/56", "quad9"], \
    [ "2001:0500:0015:1400::/56", "quad9"], \
    [ "2001:0500:0015:1700::/56", "quad9"], \
    [ "2001:0500:0015:2100::/56", "quad9"], \
    [ "2001:0500:0015:2300::/56", "quad9"], \
    [ "2001:0500:0015:2500::/56", "quad9"], \
    [ "2001:0500:0015:2900::/56", "quad9"], \
    [ "2001:0500:0015:3000::/56", "quad9"], \
    [ "2001:0500:0015:3100::/56", "quad9"], \
    [ "2001:0500:0015:3200::/56", "quad9"], \
    [ "2001:0500:0015:3300::/56", "quad9"], \
    [ "2001:0500:0015:3500::/56", "quad9"], \
    [ "2620:171:eb:f0:0:0:0:0/64", "quad9"], \
    [ "2001:0500:0015:4400::/56", "quad9"], \
    [ "2001:0500:0015:4600::/56", "quad9"], \
    [ "2001:0500:0015:4800::/56", "quad9"], \
    [ "2001:0500:0015:4900::/56", "quad9"], \
    [ "2001:0500:0015:5000::/56", "quad9"], \
    [ "2001:0500:0015:5400::/56", "quad9"], \
    [ "2001:0500:0015:5500::/56", "quad9"], \
    [ "2001:0500:0015:5700::/56", "quad9"], \
    [ "2001:0500:0015:5800::/56", "quad9"], \
    [ "2001:0500:0015:6000::/56", "quad9"], \
    [ "2001:0500:0015:6200::/56", "quad9"], \
    [ "2001:0500:0015:6300::/56", "quad9"], \
    [ "2001:0500:0015:6400::/56", "quad9"], \
    [ "2001:0500:0015:6700::/56", "quad9"], \
    [ "2001:0500:0015:6900::/56", "quad9"], \
    [ "2001:0500:0015:7000::/56", "quad9"], \
    [ "2620:0171:0071:0000::/48", "quad9"], \
    [ "2620:0000:0876:1000::/56", "quad9"], \
    [ "2620:0000:0876:1100::/56", "quad9"], \
    [ "2620:0000:0876:1200::/56", "quad9"], \
    [ "2620:0000:0876:1300::/56", "quad9"], \
    [ "2620:0000:0876:1400::/56", "quad9"], \
    [ "2620:0000:0876:1500::/56", "quad9"], \
    [ "2620:0000:0876:1600::/56", "quad9"], \
    [ "2620:0000:0876:1700::/56", "quad9"], \
    [ "2620:0000:0876:1800::/56", "quad9"], \
    [ "2620:171:f3:f0:0:0:0:0/64", "quad9"], \
    [ "2620:0000:0876:2000::/56", "quad9"], \
    [ "2620:0000:0876:2100::/56", "quad9"], \
    [ "2620:0000:0876:2200::/56", "quad9"], \
    [ "2620:0000:0876:2300::/56", "quad9"], \
    [ "2620:0000:0876:2400::/56", "quad9"], \
    [ "2620:0000:0876:2500::/56", "quad9"], \
    [ "2620:0000:0876:2600::/56", "quad9"], \
    [ "2620:0000:0876:2700::/56", "quad9"], \
    [ "2620:0000:0876:2800::/56", "quad9"], \
    [ "2620:0000:0876:2900::/56", "quad9"], \
    [ "2620:0000:0876:3000::/56", "quad9"], \
    [ "2620:0000:0876:3100::/56", "quad9"], \
    [ "2620:0000:0876:3200::/56", "quad9"], \
    [ "2620:0000:0876:3300::/56", "quad9"], \
    [ "2620:0000:0876:3400::/56", "quad9"], \
    [ "2620:0000:0876:3500::/56", "quad9"], \
    [ "2620:0000:0876:3600::/56", "quad9"], \
    [ "2620:0000:0876:3700::/56", "quad9"], \
    [ "2620:0000:0876:3800::/56", "quad9"], \
    [ "2620:0000:0876:3900::/56", "quad9"], \
    [ "2620:0000:0876:4000::/56", "quad9"], \
    [ "2620:0000:0876:4100::/56", "quad9"], \
    [ "2620:0000:0876:4200::/56", "quad9"], \
    [ "2620:0000:0876:4300::/56", "quad9"], \
    [ "2620:0000:0876:4400::/56", "quad9"], \
    [ "2620:0000:0876:4500::/56", "quad9"], \
    [ "2620:0000:0876:4600::/56", "quad9"], \
    [ "2620:0000:0876:4700::/56", "quad9"], \
    [ "2620:0000:0876:4800::/56", "quad9"], \
    [ "2620:0000:0876:4900::/56", "quad9"], \
    [ "2620:0000:0876:5000::/56", "quad9"], \
    [ "2620:0000:0876:5100::/56", "quad9"], \
    [ "2620:0000:0876:5200::/56", "quad9"], \
    [ "2620:0000:0876:5300::/56", "quad9"], \
    [ "2620:0000:0876:5400::/56", "quad9"], \
    [ "2620:0000:0876:5500::/56", "quad9"], \
    [ "2620:0000:0876:5600::/56", "quad9"], \
    [ "2620:0000:0876:5700::/56", "quad9"], \
    [ "2620:0000:0876:5800::/56", "quad9"], \
    [ "2620:0000:0876:5900::/56", "quad9"], \
    [ "2620:0000:0876:6000::/56", "quad9"], \
    [ "2620:0000:0876:6100::/56", "quad9"], \
    [ "2620:0000:0876:6200::/56", "quad9"], \
    [ "2620:0000:0876:6300::/56", "quad9"], \
    [ "2620:0000:0876:6400::/56", "quad9"], \
    [ "2620:0000:0876:6500::/56", "quad9"], \
    [ "2620:0000:0876:6600::/56", "quad9"], \
    [ "2620:0000:0876:6700::/56", "quad9"], \
    [ "2620:0000:0876:6800::/56", "quad9"], \
    [ "2620:0000:0876:6900::/56", "quad9"], \
    [ "2620:0000:0876:7000::/56", "quad9"], \
    [ "2620:0000:0876:7100::/56", "quad9"], \
    [ "2620:0000:0876:7200::/56", "quad9"], \
    [ "2620:0000:0876:7300::/56", "quad9"], \
    [ "2620:0000:0876:7400::/56", "quad9"], \
    [ "2620:0000:0876:7500::/56", "quad9"], \
    [ "2620:0000:0876:7600::/56", "quad9"], \
    [ "2620:0000:0876:7700::/56", "quad9"], \
    [ "2620:0000:0876:7800::/56", "quad9"], \
    [ "2620:0000:0876:7900::/56", "quad9"], \
    [ "2620:0000:0876:8000::/56", "quad9"], \
    [ "2620:0000:0876:8100::/56", "quad9"], \
    [ "2620:0000:0876:8200::/56", "quad9"], \
    [ "2620:0000:0876:8300::/56", "quad9"], \
    [ "2620:0000:0876:8400::/56", "quad9"], \
    [ "2620:0000:0876:8500::/56", "quad9"], \
    [ "2620:0000:0876:8600::/56", "quad9"], \
    [ "2620:0000:0876:8700::/56", "quad9"], \
    [ "2620:0000:0876:8800::/56", "quad9"], \
    [ "2620:0000:0876:8900::/56", "quad9"], \
    [ "2620:0000:0876:9000::/56", "quad9"], \
    [ "2620:0000:0876:9100::/56", "quad9"], \
    [ "2620:0000:0876:9200::/56", "quad9"], \
    [ "2620:0000:0876:9300::/56", "quad9"], \
    [ "2620:0000:0876:9400::/56", "quad9"], \
    [ "2620:0000:0876:9500::/56", "quad9"], \
    [ "2620:0000:0876:9600::/56", "quad9"], \
    [ "2620:0000:0876:9700::/56", "quad9"], \
    [ "2620:0000:0876:9800::/56", "quad9"], \
    [ "2620:0000:0876:9900::/56", "quad9"], \
    [ "2620:0171:0001:0000::/48", "quad9"], \
    [ "2620:0171:0002:0000::/48", "quad9"], \
    [ "2620:0171:0004:0000::/48", "quad9"], \
    [ "2620:0171:0006:0000::/48", "quad9"], \
    [ "2620:0171:0008:0000::/48", "quad9"], \
    [ "2620:0171:0010:0000::/48", "quad9"], \
    [ "2620:0171:0011:0000::/48", "quad9"], \
    [ "2620:0171:0013:0000::/48", "quad9"], \
    [ "2620:0171:0014:0000::/48", "quad9"], \
    [ "2620:0171:0017:0000::/48", "quad9"], \
    [ "2620:0171:0018:0000::/48", "quad9"], \
    [ "2620:0171:0023:0000::/48", "quad9"], \
    [ "2620:0171:0024:0000::/48", "quad9"], \
    [ "2620:0171:0025:0000::/48", "quad9"], \
    [ "2620:0171:0028:0000::/48", "quad9"], \
    [ "2620:0171:0030:0000::/48", "quad9"], \
    [ "2620:0171:0031:0000::/48", "quad9"], \
    [ "2620:0171:0032:0000::/48", "quad9"], \
    [ "2620:0171:0033:0000::/48", "quad9"], \
    [ "2620:0171:0034:0000::/48", "quad9"], \
    [ "2620:0171:0035:0000::/48", "quad9"], \
    [ "2620:0171:0036:0000::/48", "quad9"], \
    [ "2620:0171:0037:0000::/48", "quad9"], \
    [ "2620:0171:0038:0000::/48", "quad9"], \
    [ "2620:0171:0042:0000::/48", "quad9"], \
    [ "2620:0171:0043:0000::/48", "quad9"], \
    [ "2620:0171:0048:0000::/48", "quad9"], \
    [ "2620:0171:0049:0000::/48", "quad9"], \
    [ "2620:0171:0050:0000::/48", "quad9"], \
    [ "2620:0171:0051:0000::/48", "quad9"], \
    [ "2620:0171:0052:0000::/48", "quad9"], \
    [ "2620:0171:0053:0000::/48", "quad9"], \
    [ "2620:0171:0054:0000::/48", "quad9"], \
    [ "2620:0171:0055:0000::/48", "quad9"], \
    [ "2620:0171:0056:0000::/48", "quad9"], \
    [ "2620:0171:0057:0000::/48", "quad9"], \
    [ "2620:0171:0058:0000::/48", "quad9"], \
    [ "2620:0171:0061:0000::/48", "quad9"], \
    [ "2620:0171:0064:0000::/48", "quad9"], \
    [ "2620:0171:0065:0000::/48", "quad9"], \
    [ "2620:0171:0066:0000::/48", "quad9"], \
    [ "2620:0171:0067:0000::/48", "quad9"], \
    [ "2620:0171:0068:0000::/48", "quad9"], \
    [ "2620:0171:0074:0000::/48", "quad9"], \
    [ "2620:0171:0076:0000::/48", "quad9"], \
    [ "2620:0171:00E0:0000::/48", "quad9"], \
    [ "2620:0171:00E1:0000::/48", "quad9"], \
    [ "2620:0171:00E2:0000::/48", "quad9"], \
    [ "2620:0171:00E3:0000::/48", "quad9"], \
    [ "2620:0171:00E4:0000::/48", "quad9"], \
    [ "2620:0171:00E5:0000::/48", "quad9"], \
    [ "2620:0171:00E6:0000::/48", "quad9"], \
    [ "2620:0171:00E7:0000::/48", "quad9"], \
    [ "2620:0171:00E8:0000::/48", "quad9"], \
    [ "2620:0171:00E9:0000::/48", "quad9"], \
    [ "2620:0171:00EA:0000::/48", "quad9"], \
    [ "2620:0171:00F0:0000::/48", "quad9"], \
    [ "2620:0171:00F1:0000::/48", "quad9"], \
    [ "2620:0171:00F2:0000::/48", "quad9"], \
    [ "2620:0171:00F3:0000::/48", "quad9"], \
    [ "2620:0171:00F4:0000::/48", "quad9"], \
    [ "2620:0171:00F5:0000::/48", "quad9"], \
    [ "2620:0171:00F6:0000::/48", "quad9"], \
    [ "2620:0171:00F7:0000::/48", "quad9"], \
    [ "2620:0171:00F8:0000::/48", "quad9"], \
    [ "2620:0171:00F9:0000::/48", "quad9"], \
    [ "2620:0171:00FA:0000::/48", "quad9"], \
    [ "2620:0171:00FB:0000::/48", "quad9"], \
    [ "2620:0171:00FC:0000::/48", "quad9"], \
    [ "2620:0171:00FD:0000::/48", "quad9"], \
    [ "2620:0171:00FE:0000::/48", "quad9"], \
    [ "2620:0171:00FF:0000::/48", "quad9"], \
    [ "77.88.56.0/24", "yandex"], \
    [ "185.100.3.0/24", "vrsgn"], \
    [ "192.34.238.0/24", "vrsgn"], \
    [ "192.34.234.0/23", "vrsgn"], \
    [ "192.34.236.0/23", "vrsgn"], \
    [ "192.68.128.0/23", "vrsgn"], \
    [ "192.68.126.0/23", "vrsgn"], \
    [ "192.68.130.0/24", "vrsgn"], \
    [ "192.81.185.0/24", "vrsgn"], \
    [ "192.81.186.0/23", "vrsgn"], \
    [ "192.81.188.0/23", "vrsgn"], \
    [ "199.7.48.0/20", "vrsgn"], \
    [ "209.131.128.0/18", "vrsgn"], \
    [ "216.87.142.0/24", "vrsgn"], \
    [ "217.30.80.0/20", "vrsgn"], \
    [ "81.19.192.0/20", "vrsgn"], \
    [ "2620:74::/32", "vrsgn"], \
    [ "103.87.108.0/22", "vrsgn"], \
    [ "2402:79c0::/32", "vrsgn"], \
    [ "203.144.48.0/20", "vrsgn"], \
    [ "74.125.18.0/25", "googlepdns"], \
    [ "74.125.18.128/26", "googlepdns"], \
    [ "74.125.18.192/26", "googlepdns"], \
    [ "74.125.19.0/24", "googlepdns"], \
    [ "74.125.40.0/24", "googlepdns"], \
    [ "74.125.41.0/24", "googlepdns"], \
    [ "74.125.42.0/24", "googlepdns"], \
    [ "74.125.44.0/24", "googlepdns"], \
    [ "74.125.45.0/24", "googlepdns"], \
    [ "74.125.46.0/24", "googlepdns"], \
    [ "74.125.47.0/24", "googlepdns"], \
    [ "74.125.72.0/24", "googlepdns"], \
    [ "74.125.73.0/24", "googlepdns"], \
    [ "74.125.74.0/24", "googlepdns"], \
    [ "74.125.75.0/24", "googlepdns"], \
    [ "74.125.76.0/24", "googlepdns"], \
    [ "74.125.77.0/24", "googlepdns"], \
    [ "74.125.78.0/24", "googlepdns"], \
    [ "74.125.79.0/24", "googlepdns"], \
    [ "74.125.80.0/24", "googlepdns"], \
    [ "74.125.81.0/24", "googlepdns"], \
    [ "74.125.92.0/24", "googlepdns"], \
    [ "74.125.112.0/24", "googlepdns"], \
    [ "74.125.113.0/24", "googlepdns"], \
    [ "74.125.115.0/24", "googlepdns"], \
    [ "74.125.176.0/24", "googlepdns"], \
    [ "74.125.177.0/24", "googlepdns"], \
    [ "74.125.178.0/24", "googlepdns"], \
    [ "74.125.179.0/24", "googlepdns"], \
    [ "74.125.180.0/24", "googlepdns"], \
    [ "74.125.181.0/24", "googlepdns"], \
    [ "74.125.182.0/24", "googlepdns"], \
    [ "74.125.183.0/24", "googlepdns"], \
    [ "74.125.184.0/24", "googlepdns"], \
    [ "74.125.185.0/24", "googlepdns"], \
    [ "74.125.186.0/24", "googlepdns"], \
    [ "74.125.187.0/24", "googlepdns"], \
    [ "74.125.190.0/24", "googlepdns"], \
    [ "74.125.191.0/24", "googlepdns"], \
    [ "172.217.32.0/25", "googlepdns"], \
    [ "172.217.32.128/25", "googlepdns"], \
    [ "172.217.33.0/25", "googlepdns"], \
    [ "172.217.33.128/25", "googlepdns"], \
    [ "172.217.34.0/26", "googlepdns"], \
    [ "172.217.34.64/26", "googlepdns"], \
    [ "172.217.34.128/25", "googlepdns"], \
    [ "172.217.35.0/24", "googlepdns"], \
    [ "172.217.36.0/24", "googlepdns"], \
    [ "172.217.37.0/24", "googlepdns"], \
    [ "172.217.38.0/24", "googlepdns"], \
    [ "172.217.39.0/24", "googlepdns"], \
    [ "172.217.40.0/24", "googlepdns"], \
    [ "172.217.41.0/24", "googlepdns"], \
    [ "172.217.42.0/24", "googlepdns"], \
    [ "172.217.43.0/24", "googlepdns"], \
    [ "172.217.44.0/24", "googlepdns"], \
    [ "172.217.45.0/24", "googlepdns"], \
    [ "172.217.46.0/24", "googlepdns"], \
    [ "172.217.47.0/24", "googlepdns"], \
    [ "172.253.0.0/24", "googlepdns"], \
    [ "172.253.1.0/24", "googlepdns"], \
    [ "172.253.2.0/24", "googlepdns"], \
    [ "172.253.3.0/24", "googlepdns"], \
    [ "172.253.4.0/24", "googlepdns"], \
    [ "172.253.5.0/24", "googlepdns"], \
    [ "172.253.6.0/24", "googlepdns"], \
    [ "172.253.7.0/24", "googlepdns"], \
    [ "172.253.8.0/24", "googlepdns"], \
    [ "172.253.9.0/24", "googlepdns"], \
    [ "172.253.10.0/24", "googlepdns"], \
    [ "172.253.11.0/24", "googlepdns"], \
    [ "172.253.12.0/24", "googlepdns"], \
    [ "172.253.13.0/24", "googlepdns"], \
    [ "172.253.14.0/24", "googlepdns"], \
    [ "172.253.15.0/24", "googlepdns"], \
    [ "172.253.192.0/24", "googlepdns"], \
    [ "172.253.193.0/24", "googlepdns"], \
    [ "172.253.194.0/24", "googlepdns"], \
    [ "172.253.195.0/24", "googlepdns"], \
    [ "172.253.196.0/24", "googlepdns"], \
    [ "172.253.197.0/24", "googlepdns"], \
    [ "172.253.198.0/24", "googlepdns"], \
    [ "172.253.199.0/24", "googlepdns"], \
    [ "172.253.200.0/24", "googlepdns"], \
    [ "172.253.201.0/24", "googlepdns"], \
    [ "172.253.202.0/24", "googlepdns"], \
    [ "172.253.204.0/24", "googlepdns"], \
    [ "172.253.205.0/24", "googlepdns"], \
    [ "172.253.206.0/24", "googlepdns"], \
    [ "172.253.209.0/24", "googlepdns"], \
    [ "172.253.210.0/24", "googlepdns"], \
    [ "172.253.211.0/24", "googlepdns"], \
    [ "173.194.90.0/24", "googlepdns"], \
    [ "173.194.91.0/24", "googlepdns"], \
    [ "173.194.93.0/24", "googlepdns"], \
    [ "173.194.94.0/24", "googlepdns"], \
    [ "173.194.95.0/24", "googlepdns"], \
    [ "173.194.96.0/24", "googlepdns"], \
    [ "173.194.97.0/24", "googlepdns"], \
    [ "173.194.98.0/24", "googlepdns"], \
    [ "173.194.99.0/24", "googlepdns"], \
    [ "173.194.100.0/24", "googlepdns"], \
    [ "173.194.101.0/24", "googlepdns"], \
    [ "173.194.102.0/24", "googlepdns"], \
    [ "173.194.103.0/24", "googlepdns"], \
    [ "173.194.168.0/25", "googlepdns"], \
    [ "173.194.168.128/26", "googlepdns"], \
    [ "173.194.168.192/26", "googlepdns"], \
    [ "173.194.169.0/24", "googlepdns"], \
    [ "173.194.170.0/24", "googlepdns"], \
    [ "173.194.171.0/24", "googlepdns"], \
    [ "2404:6800:4000::/48", "googlepdns"], \
    [ "2404:6800:4003::/48", "googlepdns"], \
    [ "2404:6800:4005::/48", "googlepdns"], \
    [ "2404:6800:4006::/48", "googlepdns"], \
    [ "2404:6800:4008::/48", "googlepdns"], \
    [ "2404:6800:400a::/48", "googlepdns"], \
    [ "2404:6800:400b::/48", "googlepdns"], \
    [ "2404:6800:4013::/48", "googlepdns"], \
    [ "2607:f8b0:4001::/48", "googlepdns"], \
    [ "2607:f8b0:4002::/48", "googlepdns"], \
    [ "2607:f8b0:4003::/48", "googlepdns"], \
    [ "2607:f8b0:4004::/52", "googlepdns"], \
    [ "2607:f8b0:4004:1000::/52", "googlepdns"], \
    [ "2607:f8b0:400c::/48", "googlepdns"], \
    [ "2607:f8b0:400d::/48", "googlepdns"], \
    [ "2607:f8b0:400e::/48", "googlepdns"], \
    [ "2607:f8b0:4020::/48", "googlepdns"], \
    [ "2607:f8b0:4023::/48", "googlepdns"], \
    [ "2800:3f0:4001::/48", "googlepdns"], \
    [ "2800:3f0:4003::/48", "googlepdns"], \
    [ "2a00:1450:4001::/48", "googlepdns"], \
    [ "2a00:1450:4009::/48", "googlepdns"], \
    [ "2a00:1450:400a::/48", "googlepdns"], \
    [ "2a00:1450:400b::/48", "googlepdns"], \
    [ "2a00:1450:400c::/48", "googlepdns"], \
    [ "2a00:1450:4010::/48", "googlepdns"], \
    [ "2a00:1450:4013::/48", "googlepdns"], \
    [ "2a00:1450:4025::/48", "googlepdns"], \
    [ "146.112.128.0/21", "opendns"], \
    [ "146.112.136.0/23", "opendns"], \
    [ "146.112.138.0/24", "opendns"], \
    [ "185.60.86.0/23", "opendns"], \
    [ "204.194.237.0/24", "opendns"], \
    [ "204.194.238.0/24", "opendns"], \
    [ "204.194.239.0/24", "opendns"], \
    [ "208.67.216.0/24", "opendns"], \
    [ "208.67.217.0/24", "opendns"], \
    [ "208.67.219.0/24", "opendns"], \
    [ "208.69.32.0/24", "opendns"], \
    [ "208.69.33.0/24", "opendns"], \
    [ "208.69.34.0/24", "opendns"], \
    [ "208.69.35.0/24", "opendns"], \
    [ "208.69.36.0/24", "opendns"], \
    [ "208.69.37.0/24", "opendns"], \
    [ "2620:0:cc3::/48", "opendns"], \
    [ "2620:0:cc4::/48", "opendns"], \
    [ "2620:0:cc5::/48", "opendns"], \
    [ "2620:0:cc6::/48", "opendns"], \
    [ "2620:0:cc7::/48", "opendns"], \
    [ "2620:0:cc8::/48", "opendns"], \
    [ "2620:0:cc9::/48", "opendns"], \
    [ "2620:0:cca::/48", "opendns"], \
    [ "2620:0:ccb::/48", "opendns"], \
    [ "2620:0:cce::/48", "opendns"], \
    [ "2620:0:ccf::/48", "opendns"], \
    [ "2620:119:10::/48", "opendns"], \
    [ "2620:119:11::/48", "opendns"], \
    [ "2620:119:12::/48", "opendns"], \
    [ "2620:119:13::/48", "opendns"], \
    [ "2a04:e4c0:10::/48", "opendns"], \
    [ "2a04:e4c0:12::/48", "opendns"], \
    [ "2a04:e4c0:14::/48", "opendns"], \
    [ "2a04:e4c0:15::/48", "opendns"], \
    [ "2a04:e4c0:16::/48", "opendns"], \
    [ "2a04:e4c0:17::/48", "opendns"], \
    [ "2a04:e4c0:18::/48", "opendns"], \
    [ "2a04:e4c0:20::/48", "opendns"], \
    [ "2a04:e4c0:21::/48", "opendns"], \
    [ "2a04:e4c0:22::/48", "opendns"], \
    [ "2a04:e4c0:23::/48", "opendns"], \
    [ "2a04:e4c0:24::/48", "opendns"], \
    [ "2a04:e4c0:25::/48", "opendns"], \
    [ "2a04:e4c0:30::/48", "opendns"], \
    [ "2a04:e4c0:31::/48", "opendns"], \
    [ "2a04:e4c0:40::/48", "opendns"], \
    [ "67.215.80.0/24", "opendns"], \
    [ "67.215.82.0/24", "opendns"], \
    [ "67.215.83.0/24", "opendns"], \
    [ "67.215.84.0/24", "opendns"], \
    [ "67.215.85.0/24", "opendns"], \
    [ "67.215.86.0/24", "opendns"], \
    [ "121.40.12.0/24", "cnnic"], \
    [ "2001:dc7::/32", "cnnic"], \
    [ "203.119.33.0/24", "cnnic"], \
    [ "211.144.10.0/24", "cnnic"], \
    [ "42.83.200.0/24", "cnnic"], \
    [ "54.153.39.191/32", "yandex-amazon"], \
    [ "192.221.0.0/16", "level3"], \
    [ "8.0.0.0/16", "level3"], \
    [ "184.105.250.0/23", "he"], \
    [ "184.105.252.0/22", "he"], \
    [ "2001:470:0::/48", "he"], \
    [ "209.51.161.0/24", "he"], \
    [ "216.218.128.0/17", "he"], \
    [ "216.66.0.0/18", "he"], \
    [ "216.66.64.0/19", "he"], \
    [ "64.62.128.0/17", "he"], \
    [ "66.220.0.0/19", "he"], \
    [ "72.52.64.0/18", "he"], \
    [ "74.82.0.0/18", "he"], \
    [ "AS13335", "cloudflare"], \
    [ "AS204136", "opennic"], \
    [ "AS12008", "neustar"], \
    [ "AS15169", "googlepdns"], \
    [ "AS4812", "dnspai"], \
    [ "AS23274", "dnspai"], \
    [ "AS132203", "dnspod"], \
    [ "AS131400", "dnswatch"], \
    [ "AS31400", "dnswatch"], \
    [ "AS33517", "dyn"], \
    [ "AS51453", "freedns"], \
    [ "AS60679", "freedomworld"], \
    [ "AS8551", "greenteamdns"], \
    [ "AS1680", "greenteamdns"], \
    [ "AS4808", "onedns"], \
    [ "AS23724", "onedns"], \
    [ "AS204136", "opennic"], \
    [ "AS57926", "safedns"], \
    [ "AS131621", "twnic"], \
    [ "130.225.244.166/32", "uncensoreddns"], \
    [ "130.226.161.34/32", "uncensoreddns"], \
    [ "2001:878:0:e000::/64", "uncensoreddns"], \
    [ "2001:878:0:e000::/64", "uncensoreddns"], \
    [ "2a01:3a0:53:53::/64", "uncensoreddns"], \
    [ "89.233.43.71/32", "uncensoreddns"], \
    [ "AS23393", "comodo"], \
    [ "176.56.180.0/24", "safedns"], \
    [ "176.56.185.0/24", "safedns"], \
    [ "185.48.57.0/24", "safedns"], \
    [ "103.200.218.0/24", "freenom"], \
    [ "184.170.249.0/24", "freenom"], \
    [ "219.99.143.0/24", "freenom"], \
    [ "2406:f400:22:103::/48", "freenom"], \
    [ "2607:f7a0:1:3::/48", "freenom"], \
    [ "2a00:ec8:400:ff04::/48", "freenom"], \
    [ "2a00:f80:56::/48", "freenom"], \
    [ "83.223.39.0/24", "freenom"], \
    [ "83.223.44.0/24", "freenom"], \
    [ "83.223.45.0/24", "freenom"], \
    [ "83.223.50.0/24", "freenom"], \
    [ "85.255.211.0/24", "freenom"], \
    [ "87.255.36.0/24", "freenom"], \
    [ "103.200.96.215/32", "cleanbrowsing"], \
    [ "103.205.140.168/32", "cleanbrowsing"], \
    [ "104.223.91.42/32", "cleanbrowsing"], \
    [ "139.180.222.255/32", "cleanbrowsing"], \
    [ "140.82.14.217/32", "cleanbrowsing"], \
    [ "140.82.30.53/32", "cleanbrowsing"], \
    [ "140.82.39.20/32", "cleanbrowsing"], \
    [ "141.98.90.234/32", "cleanbrowsing"], \
    [ "144.202.108.20/32", "cleanbrowsing"], \
    [ "149.28.187.17/32", "cleanbrowsing"], \
    [ "154.16.135.216/32", "cleanbrowsing"], \
    [ "167.179.94.245/32", "cleanbrowsing"], \
    [ "170.81.42.58/32", "cleanbrowsing"], \
    [ "185.170.209.30/32", "cleanbrowsing"], \
    [ "185.173.25.88/32", "cleanbrowsing"], \
    [ "185.181.61.4/32", "cleanbrowsing"], \
    [ "185.99.133.41/32", "cleanbrowsing"], \
    [ "196.251.250.197/32", "cleanbrowsing"], \
    [ "199.247.15.163/32", "cleanbrowsing"], \
    [ "207.148.11.35/32", "cleanbrowsing"], \
    [ "213.183.41.238/32", "cleanbrowsing"], \
    [ "213.183.63.23/32", "cleanbrowsing"], \
    [ "213.226.68.68/32", "cleanbrowsing"], \
    [ "23.236.73.15/32", "cleanbrowsing"], \
    [ "45.76.171.37/32", "cleanbrowsing"], \
    [ "45.76.58.192/32", "cleanbrowsing"], \
    [ "45.77.231.112/32", "cleanbrowsing"], \
    [ "66.154.113.154/32", "cleanbrowsing"], \
    [ "72.11.132.178/32", "cleanbrowsing"], \
    [ "91.201.65.18/32", "cleanbrowsing"], \
    [ "95.179.130.14/32", "cleanbrowsing"], \
    [ "23.253.163.53/32", "alternatedns"], \
    [ "198.101.242.82/32", "alternatidns"], \
    [ "2001:4801:7825:103::/48", "alternatedns"], \
    [ "2001:4800:780e:510::/48", "alternatedns"], \
    [ "109.69.8.51/32", "puntcat"], \
    [ "2a00:1508:0:4::/64", "puntcat"], \
    [ "198.11.178.0/23", "alidns"], \
    [ "47.252.81.0/24", "alidns"], \
    [ "47.252.85.0/24", "alidns"], \
    [ "180.149.143.0/24", "baidu"], \
    [ "180.76.14.0/24", "baidu"], \
    [ "61.135.186.0/24", "baidu"], \
    [ "58.217.249.0/24", "114dns"], \
    [ "60.215.138.0/24", "114dns"], \
    [ "203.75.51.0/24", "quad101"], \
]

# table_print: generate the tables of ASes and prefixes for the
# list of open DNS resolver. The tables are generated once from
# a file provided by APNIC, and then 
def table_print(rsvs_ips, f_name):
    as_list = dict()
    net_6 = []
    net_4 = []
    for line in open(rsvs_ips, "r"):
        try:
            line = line.strip()
            op_net = line.split(" ")
            if op_net[0].startswith('AS'):
                as_list[op_net[0]] = op_net[1]
            else:
                ip_net = ipaddress.ip_network(op_net[0], strict=False)
                net_range = [ ip_net, op_net[1] ]
                if ip_net.version == 4:
                    net_4.append(net_range)
                else:
                    net_6.append(net_range)
        except Exception as exc:
            traceback.print_exc()
            print('\nCode generated an exception: %s' % (exc))
            print("Cannot parse:\n" + line + "\n")

    net_4_sorted = sorted(net_4, key=lambda x: x[0])
    net_6.sort(key = lambda x: x[0])

    with open(f_name, "w") as F:
        F.write("as_table = [\n")
        for asn in as_list:
            F.write("    [\"" + asn + "\",\"" + as_list[asn] + "\"],\n")
        F.write("]\n")
        F.write("n4_table = [\n")
        for n4 in net_4_sorted:
             F.write("    [ipaddress.ip_network(\"" + str(n4[0]) + "\"),\"" + n4[1] + "\"],\n")
        F.write("]\n")
        F.write("n6_table = [\n")
        for n6 in net_6:
             F.write("    [ipaddress.ip_network(\"" + str(n6[0]) + "\"),\"" + n6[1] + "\"],\n")
        F.write("]\n")
