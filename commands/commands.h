
//  название, номер, есть ли аргументы, что делать

#define PUSH(arg) stack_push(&(cpu->stack), arg)

#define POP       stack_pop(&(cpu->stack))

#define PUSH_CALL(arg) stack_push(&(cpu->calls), arg)

#define POP_CALL       stack_pop(&(cpu->calls))

#define check_stack_size_ans_error(num, funck)  if (cpu->stack.size < num) {                                                \
                                                    printf("ERROR in "#funck" ip:%d, stack size < "#num"\n", (cpu->ip)-2);  \
                                                    end_flug = true;                                                        \
                                                    break;                                                                  \
                                                }

DEF_CMD(push, 1, 1, {   int *arg = get_ptr_arg(cpu);

                        PUSH(*arg);
                    })

DEF_CMD(add, 2, 0, {    check_stack_size_ans_error(2, add);

                        PUSH(POP + POP);
                    })

DEF_CMD(sup, 3, 0, {    check_stack_size_ans_error(2, sup);

                        int b = POP;
                        int a = POP;

                        PUSH(a - b);
                    })

DEF_CMD(mul, 4, 0, {    check_stack_size_ans_error(2, nul);

                        PUSH(POP * POP);
                    })

DEF_CMD(div, 5, 0, {    check_stack_size_ans_error(2, div)

                        int b = POP;
                        int a = POP;

                        if (b == 0) {
                            PUSH(std::numeric_limits<int>::max());
                        }
                        else {
                            PUSH(a / b);
                        }
                    })

DEF_CMD(out, 6, 0, {    check_stack_size_ans_error(1, out)

                        printf("%d\n", POP);
                    })

DEF_CMD(hell, 7, 0, {   end_flug = true;
                        break;
                    })

DEF_CMD(jmp, 8, 1, {    int *arg = get_ptr_arg(cpu);

                        cpu->ip = *arg;
                    })

DEF_CMD(pop, 9, 1, {    int *arg = get_ptr_arg(cpu);

                        *arg = POP;
                    })

DEF_CMD(call, 10, 1, {  int *arg = get_ptr_arg(cpu);

                        PUSH_CALL((int)cpu->ip);
                        
                        cpu->ip = *arg;
                    })

DEF_CMD(ret, 11, 0, {   if (cpu->calls.size == 0) {
                            printf("error ret ip:%d\n", cpu->ip);
                            end_flug = true;
                            break;
                        }
                        cpu->ip = POP_CALL;
                    })

DEF_CMD(jb, 12, 1, {    int a = 0;
                        int b = 0;
                        int arg = 0;

                        get_jmp_param(&a, &b, &arg, cpu);
    
                        if (a < b) {
                            cpu->ip = arg;
                        }
                    })

#define CALC_W (a % (MEM_W * PIXEL_SIZE))

#define CALC_H (a / (MEM_W * PIXEL_SIZE)) % (MEM_H * PIXEL_SIZE)

DEF_CMD(drom, 13, 0, {  if (window.isOpen()) {
                            for ( int a = 0; a < 320*4 * 200*4 ; a++) {
                                pointmap[a].position = sf::Vector2f((float)(CALC_W), (float)(CALC_H));
                                pointmap[a].color = (sf::Color)(cpu->RAM[CALC_W / PIXEL_SIZE + CALC_H / PIXEL_SIZE * MEM_W]);
                            }
                            window.clear();
                            window.draw(pointmap);

                            window.display();
                        }
                     })

DEF_CMD(sqrt, 14, 0, {  check_stack_size_ans_error(1, sqrt)

                        PUSH((int)sqrt(POP));
                     })

DEF_CMD(je, 15, 1, {    int a = 0;
                        int b = 0;
                        int arg = 0;

                        get_jmp_param(&a, &b, &arg, cpu);
    
                        if (a == b) {
                            cpu->ip = arg;
                        }
                    })