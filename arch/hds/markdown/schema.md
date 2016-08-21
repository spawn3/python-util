
# protection domain

PD:

- id
- name

Node:

- id
- pdid
- hostname
- ip

User:

- id
- name
- create time
- rolelist

Permission:

- id
- name

Role:

- id
- name
- permissionid

Pool

- id
- name
- uid
- pdid
- create time

Volume

- id
- name
- pdid
- poolid
- create time
- status
- size

Snapshot

- id
- name
- type
- volumeid
- create time

CLI会绕过书记，如何处理？
