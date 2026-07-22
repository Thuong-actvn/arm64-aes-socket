#!/usr/bin/env bash
set -e

cd "$(dirname "$0")"

KERNEL="Image"
ROOTFS="core-image-sato-qemuarm64.rootfs.qcow2"

command -v qemu-system-aarch64 >/dev/null || {
    echo "Loi: chua cai qemu-system-aarch64"
    exit 1
}

[ -f "$KERNEL" ] || { echo "Loi: khong tim thay $KERNEL"; exit 1; }
[ -f "$ROOTFS" ] || { echo "Loi: khong tim thay $ROOTFS"; exit 1; }

qemu-system-aarch64 \
    -machine virt \
    -cpu cortex-a57 \
    -smp 4 \
    -m 1024 \
    -kernel "$KERNEL" \
    -drive "file=$ROOTFS,format=qcow2,if=virtio" \
    -netdev user,id=net0,hostfwd=tcp:127.0.0.1:2222-:22,hostfwd=tcp:127.0.0.1:8888-:8888 \
    -device virtio-net-pci,netdev=net0 \
    -device virtio-gpu-pci \
    -device qemu-xhci \
    -device usb-tablet \
    -device usb-kbd \
    -append "root=/dev/vda rw console=ttyAMA0"

