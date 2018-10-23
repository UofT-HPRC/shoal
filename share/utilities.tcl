proc lremove {list args} {
    foreach toRemove $args {
        set index [lsearch -exact $list $toRemove]
        set list [lreplace $list $index $index]
    }
    return $list
}
