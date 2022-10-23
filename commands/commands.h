
//  название, номер, есть ли аргументы, что делать

DEF_CMD(push, 1, 1, {   int *arg = get_ptr_arg(program, REG, RAM, &ip);

                        stack_push(&stack, *arg);

                        break;
                    })

DEF_CMD(add, 2, 0, {    if (stack.size < 2) {
                            printf("ERROR in add ip:%d, stack size < 2\n", ip-2);
                            return 0;
                        }

                        stack_push(&stack, stack_pop(&stack) + stack_pop(&stack));

                        break;
                    })

DEF_CMD(sup, 3, 0, {    if (stack.size < 2) {
                            printf("ERROR in sup ip:%d, stack size < 2\n", ip-2);
                            return 0;
                        }

                        int b = stack_pop(&stack);
                        int a = stack_pop(&stack);

                        stack_push(&stack, a - b);

                        break;
                    })

DEF_CMD(mul, 4, 0, {    if (stack.size < 2) {
                            printf("ERROR in mul ip:%d, stack size < 2\n", ip-2);
                            return 0;
                        }

                        stack_push(&stack, stack_pop(&stack) * stack_pop(&stack));

                        break;
                    })

DEF_CMD(div, 5, 0, {    if (stack.size < 2) {
                            printf("ERROR in div ip:%d, stack size < 2\n", ip-2);
                            return 0;
                        }

                        int b = stack_pop(&stack);
                        int a = stack_pop(&stack);

                        if (b == 0) {
                            stack_push(&stack, 2147483647);
                        }
                        else {
                            stack_push(&stack, a / b);
                        }

                        break;
                    })

DEF_CMD(out, 6, 0, {    if (stack.size < 1) {
                            printf("ERROR in out ip:%d, stack is empty\n", ip-2);
                            return 0;
                        }

                        printf("%d\n", stack_pop(&stack));

                        break;
                    })

DEF_CMD(hell, 7, 0, {   printf("END PROGRAM\n");
                        return 0;
                        break;
                    })

DEF_CMD(jmp, 8, 1, {    char cmd_param = program[ip++];
                        int arg = 0;
                        char reg = 0;
                        int inte = 0;

                        if (cmd_param & REG_MASK) {
                            reg = program[ip++];
                            if (reg < 1 || reg > 4) {
                                printf("error in reg number, number reg is %d", reg);
                                return 0;
                            }

                            arg = REG[reg];
                        }
                
                        if (cmd_param & INT_MASK) {
                            for (size_t i = 0; i < sizeof(int); i++) {
                                ((char *)(&inte))[i] = program[ip++];
                            }

                            if (cmd_param & REG_MASK) {
                                arg += inte;
                            }
                            else {
                                arg = inte;
                            }
                        }

                        if (cmd_param & MEM_MASK) {
                            arg = RAM[arg];
                        }

                        ip = arg;

                        break;
                    })

DEF_CMD(pop, 9, 1, {    int *arg = get_ptr_arg(program, REG, RAM, &ip);

                        *arg = stack_pop(&stack);

                        break;
                    })

DEF_CMD(call, 10, 1, {  char cmd_param = program[ip++];
                        int inte = 0;

                        if (cmd_param & REG_MASK) {
                            inte  += (unsigned char)REG[program[ip++]];
                        }

                        if (cmd_param & INT_MASK) {
                            int inte_read = 0;
                            for (size_t i = 0; i < sizeof(int); i++) {
                                ((char *)(&inte_read))[i] = program[ip++];
                            }

                            inte += inte_read;
                        }

                        stack_push(&calls, ip);
                        
                        ip = inte;
                        break;
                    })

DEF_CMD(ret, 11, 0, {if (calls.size == 0) {printf("error ret ip:%d", ip);return 0;}ip = stack_pop(&calls);break;})

DEF_CMD(jb, 12, 1, {    int a = 0;
                        int b = 0;
                        int arg = 0;

                        get_jmp_param(&ip, &a, &b, &arg, program, REG, RAM, &stack);
    
                        if (a < b) {
                            ip = arg;
                        }

                        break;
                    })

DEF_CMD(drom, 13, 0, {  /*for (int i = 0; i < 1000000; i++) {
                            if (i%1000==0) {
                                printf("\n");
                            }
                            if (RAM[i]) {
                                printf("*");
                            }
                            else{
                                printf(".");
                            }
                            
                        }

                        printf("\n");*/
                        if (window.isOpen()) {
                            for (register int a = 0; a < 320*4 * 200*4 ; a++) {
                                pointmap[a].position = sf::Vector2f((a % (320 * 4)), ((a/(320 * 4))%(200 * 4)));
                                pointmap[a].color = (sf::Color)(RAM[a % (320 * 4) / 4 + ((a/(320 * 4))%(200 * 4)) / 4 * 320]);
                            }
                            window.clear();
                            window.draw(pointmap);

                            window.display();
                        }

                        break;
                     })

DEF_CMD(sqrt, 14, 0, {  if (stack.size < 1) {
                            printf("stack empty, sqrt, ip:%d", ip);
                            return 0;
                        }

                        stack_push(&stack, sqrt(stack_pop(&stack)));

                        break;
                     })

DEF_CMD(je, 15, 1, {    int a = 0;
                        int b = 0;
                        int arg = 0;

                        get_jmp_param(&ip, &a, &b, &arg, program, REG, RAM, &stack);
    
                        if (a == b) {
                            ip = arg;
                        }

                        break;
                    })