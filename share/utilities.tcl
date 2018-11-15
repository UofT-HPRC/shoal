# buggy if args > 1? It seems to treat the two elements in the list
# as one single large element and then fails to remove it. Not quite sure, 
# why it's behaving this way. Though this could be another bug in tcl
proc lremove {list args} {
    foreach toRemove $args {
        puts $toRemove
        set index [lsearch -exact $list $toRemove]
        puts $index
        set list [lreplace $list $index $index]
    }
    return $list
}

proc listcomp {a b} {
    set res {}
    foreach item $a {
        if {$item ni $b} {
            lappend res $item
        }
    }
    return $res
 }