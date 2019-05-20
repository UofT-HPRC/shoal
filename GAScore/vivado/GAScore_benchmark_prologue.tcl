reset_target all [get_files GAScore_benchmark.bd]
export_ip_user_files -of_objects  [get_files GAScore_benchmark.bd] -sync -no_script -force -quiet
generate_target synthesis [get_files GAScore_benchmark.bd]
