# Jellyfin

I set up Jellyfin in my home lab to host and stream media that I have legally acquired on my own hardware. The main goal is to have control over my media library rather than relying entirely on streaming services and their changing catalogs.

## Why I'm Using It

- Stream my own media to different devices
- Keep my media library on hardware I control
- Organize my files into a single library
- Avoid relying on subscriptions for media I already own
- Learn more about self-hosted services and Docker

## Docker Setup

Jellyfin runs in Docker with persistent storage for its configuration and cache. My media directory is mounted into the container so Jellyfin can manage and organize the library.

The container also runs under a non-root UID/GID.

## What I'm Learning

This project has given me hands-on experience with:

- Docker and Docker Compose
- Linux file permissions
- Persistent storage
- Container networking
- Self-hosted service administration
- Managing a service that runs continuously on my home network