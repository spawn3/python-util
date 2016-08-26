# RBAC

permission:

- id
- res_type
- res_id
- op              (CRUD etc)
- ctime

role:

- id
- name
- ctime

user:

- id
- name
- password
- ctime

role-permission relationship: (id, role_id, perm_id, status, ctime)

user-role relationship: (id, user_id, role_id, ctime)

# protection domain

node:

- id
- hostname
- ip
- pdid
- ctime

pdomain:

- id
- name
- ctime

# Logical Resources

pool

- id
- name
- ctime
- uid
- pdid

volume

- id
- name
- ctime
- uid
- pdid
- poolid
- status
- size

snapshot

- id
- name
- ctime
- type
- volumeid

CLI会绕过书记，如何处理？
