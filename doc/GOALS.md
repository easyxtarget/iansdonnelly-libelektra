## Goals ##

- improve robustness
  - reject invalid configuration
  - avoid common programming errors by using better bindings
- allow software to be better integrated on configuration level
- postpone some decisions from programmers to
  maintainers/administrators:
  - syntax of the configuration files
  - side effects (e.g. logging, vcs commit, notifications)
  - flexible adoption to specific needs
  - adoption of standards (xdg, xml, json)
- reduce duplication of code (a single parser/generator used by
  everyone accessing a specific part of configuration)


## Target ##

- Embedded: Elektra is on the frontier for embedded systems because of
  its tiny core and the many possibilities with it`s plugins.
- Server: Elektra is ideal suited for a local configuration storage by
  mounting existing configuration files into the global tree. Nodes
  using Elektra can be connected by already existing configuration
  management tools.
- Desktop: Elektra allows applications to read and write from a global
  configuration tree. Missing is a description (schema) so that these
  values actually can be shared.

## Quality Goals ##

1.) Extensibility

There are so many variants of
- storage formats
- frontend integrations
- bindings

Nearly every aspect of Elektra must be extremely extensible.
On the other side semantics must be very clear and well defined
so that this extensible system works reproducible and predictable.

Only key/value pairs are the common factor and a way to get and set
them, everything else is an extension.



2.) Simplicity

An overly complex system cannot be managed nor understood.
Extensibility brings some complex issues,
which need to be solved - but in a way so that the user
sees either nothing of it or only needs to understand very
simple concepts so that it works flawlessly.
Special care for simplicity is taken for the users:
- Endusers when reconfiguring or upgrading
  should never take any notice of Elektra, except that
  it works more robust, better integrated and with less problems.
- Programmers should have multiple ways to take advantage of
  Elektra so that it flawlessly integrate with their system.
- Plugin Programmers: it should be simple to extend Elektra
  in any desired way.
- Application`s Maintainers to correctly setup+upgrade KDB
- Administrators that want to change the maintainers setup
  according to their needs


3.) Performance

Configuration is the main impact for bootup and startup time.
Elektra needs to be similar fast then current solutions.
Ideally it should get faster because of centralized optimization
endeavours where everyone using Elektra can benefit from.

Only pay for what you need.
