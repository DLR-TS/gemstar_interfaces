#!/usr/bin/env bash
# Copyright (c) 2026 DLR e.V.
#
# This program and the accompanying materials are made
# available under the terms of the Eclipse Public License 2.0
# which is available at https://www.eclipse.org/legal/epl-2.0/
#
# SPDX-License-Identifier: EPL-2.0

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
IMAGE_TAG="${IMAGE_TAG:-gemstar:gemstar_interfaces_thirdparty}"
PLUGIN_DIR="$ROOT_DIR/ThirdParty"

docker build -t "$IMAGE_TAG" "$ROOT_DIR"

container_id="$(docker create "$IMAGE_TAG")"
cleanup() {
    docker rm -f "$container_id" >/dev/null 2>&1 || true
}
trap cleanup EXIT

mkdir -p "$PLUGIN_DIR/protobuf" "$PLUGIN_DIR/osi3/include/osi3" "$PLUGIN_DIR/osi3/lib/osi3"

docker cp "$container_id:/home/ue4/thirdparty/protobuf/include/." "$PLUGIN_DIR/protobuf/include/"
docker cp "$container_id:/home/ue4/thirdparty/protobuf/lib/." "$PLUGIN_DIR/protobuf/lib/"
docker cp "$container_id:/home/ue4/UnrealEngine/osi-cpp/build/include/osi3/." "$PLUGIN_DIR/osi3/include/osi3/"
docker cp "$container_id:/home/ue4/UnrealEngine/osi-cpp/build/libopen_simulation_interface_pic.a" "$PLUGIN_DIR/osi3/lib/osi3/"

printf 'Built %s and refreshed Linux third-party artefacts.\n' "$IMAGE_TAG"
