#!/usr/bin/python
'''
* Copyright (c) 2015 Dell Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License"); you may
* not use this file except in compliance with the License. You may obtain
* a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
*
* THIS CODE IS PROVIDED ON AN  *AS IS* BASIS, WITHOUT WARRANTIES OR
* CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
* LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS
* FOR A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.
*
* See the Apache Version 2.0 License for specific language governing
* permissions and limitations under the License.
'''
import cps


class CPSTransaction:

    def __init__(self, list_of_op_obj_pairs=[]):
        """
        Constructor for transaction object
        @list_of_op_obj_pairs - list of tupples which contains the type of
                                operation("create","set","delete","rpc")
                                and the object. Based on type of operation
                                it will add the object to trascation list
        """
        self.tr_list = []
        if not isinstance(list_of_op_obj_pairs, list):
            raise ValueError("Needs List of (operation, obj) pairs")

        for pair in list_of_op_obj_pairs:
            if not isinstance(pair, tuple):
                raise ValueError("Needs List of (operation, obj) pairs")
            op = pair[0]
            op_map = {
                'create': self.create,
                'delete': self.delete,
                'set': self.set,
                'rpc': self.rpc}
            if op not in op_map:
                raise ValueError(
                    "Invalid operation - should be 'create', 'set' or 'delete'")
            op_map[pair[0]](pair[1])

    def create(self, obj):
        """
        Add an object to transaction list with "create" operation
        @obj - object to be added to the transaction
        """
        tr_obj = {}
        tr_obj['change'] = obj
        tr_obj['operation'] = "create"
        self.tr_list.append(tr_obj)

    def delete(self, obj):
        """
        Add an object to transaction list with "delete" operation
        @obj - object to be added to the transaction
        """
        tr_obj = {}
        tr_obj['change'] = obj
        tr_obj['operation'] = "delete"
        self.tr_list.append(tr_obj)

    def set(self, obj):
        """
        Add an object to transaction list with "set" operation
        @obj - object to be added to the transaction
        """
        tr_obj = {}
        tr_obj['change'] = obj
        tr_obj['operation'] = "set"
        self.tr_list.append(tr_obj)

    def rpc(self, obj):
        """
        Add an object to transaction list with "rpc" operation
        @obj - object to be added to the transaction
        """
        tr_obj = {}
        tr_obj['change'] = obj
        tr_obj['operation'] = "rpc"
        self.tr_list.append(tr_obj)

    def commit(self):
        """
        Commit the transaction list and return the response
        """
        if cps.transaction(self.tr_list):
            return self.tr_list
        return False
