from brachiograph import BrachioGraph

custom_bounds = [-7, 5, 5, 14]

bg = BrachioGraph(
    servo_1_parked_pw=1500,
    servo_2_parked_pw=1450,
    hysteresis_correction_1=10,
    hysteresis_correction_2=10,
    servo_1_degree_ms=-10.56,
    servo_2_degree_ms=10.56,
    bounds=custom_bounds
    )
