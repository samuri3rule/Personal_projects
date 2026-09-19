# Technitium DNS

I set up Technitium DNS in my home lab as a network-wide DNS server and ad blocker. I chose Technitium because it is free, self-hosted, and gives me more control over DNS configuration than a basic ad-blocking solution.

I originally planned to use Pi-hole, but since I was not limited to running the DNS server on a Raspberry Pi, I decided to use Technitium and take advantage of the additional resources available on my home lab hardware.

## Why I'm Using It

- Network-wide ad blocking
- Custom DNS rules and blocklists
- DNS query logging and statistics
- Configurable DNS forwarding
- Web-based management
- Self-hosted and free

## Docker Setup

Technitium runs in Docker with persistent volumes for configuration and logs. I also configured log retention and Docker log rotation to prevent logs from unnecessarily consuming storage.

The administrator password is passed through an environment variable rather than being stored in the Compose file.

## What I'm Learning

This project has given me hands-on experience with:

- DNS and network configuration
- Docker and Docker Compose
- Linux service administration
- Logging and storage management
- Running infrastructure that is used by other devices on my network