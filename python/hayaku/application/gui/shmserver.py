#!/usr/bin/python
# -*- coding: utf8 -*-
#
# Create on: 2026-09-07
#    Author: fasiondog

import time

import click

from hayaku.application.session import open_session


@click.command()
@click.option('--datadir', default="", help='数据目录，为空时使用 hayaku.ini 中 [hayaku] datadir')
@click.option('--publish_shm', default=True, type=bool, help='是否发布共享内存快照（K线热数据 + 基础信息）')
@click.option('--recv_spot', default=True, type=bool, help='本进程是否接收实时行情并镜像写入快照尾部')
@click.option('--config', 'config_file', default="", help='指定 hayaku 配置文件路径，为空则使用默认 ~/.hayaku/hayaku.ini')
def main(datadir, publish_shm, recv_spot, config_file):
    """在当前进程内启动 shm(共享内存)数据服务（独立 VIP 插件，需有效授权），常驻供其他 hayaku 进程零拷贝读取。

    服务不会自动产生：需在本进程显式启动；其他进程显式开启 use_shm_server=True 且同一
    datadir 已有服务时，将自动作为客户端接入。按 Ctrl-C 停止服务。
    """
    # 按 --config（或默认配置）显式打开数据会话；普通 import 不初始化数据。
    from hayaku.extensions.realtime import start_shm_server, stop_shm_server

    with open_session(config_file or None) as session:
        session.wait_ready()
        try:
            if not start_shm_server(datadir, publish_shm=publish_shm, recv_spot=recv_spot):
                click.echo("start_shm_server 返回 False：插件缺失 / 授权无效或数据未就绪，本进程未成为服务端。", err=True)
                return

            click.echo("shm 数据服务已启动，按 Ctrl-C 停止。")
            while True:
                try:
                    time.sleep(1)
                except KeyboardInterrupt:
                    break
        finally:
            stop_shm_server()


if __name__ == "__main__":
    main()
