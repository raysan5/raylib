#!/usr/bin/env python3

import json
import sys

# Return whether SARIF file contains error-level results
def codeql_sarif_contain_error(filename):
    with open(filename, 'r') as f:
        s = json.load(f)

    for run in s.get('runs', []):
        rules_metadata = run['tool']['driver']['rules']
        if not rules_metadata:
            rules_metadata = run['tool']['extensions'][0]['rules']

        for res in run.get('results', []):
            if 'ruleIndex' in res:
                rule_index = res['ruleIndex']
            elif 'rule' in res and 'index' in res['rule']:
                rule_index = res['rule']['index']
            else:
                continue
            try:
                rule_level = rules_metadata[rule_index]['defaultConfiguration']['level']
            except IndexError as e:
                print(e, rule_index, len(rules_metadata))
            else:
                if rule_level == 'error':
                    return True
    return False

if __name__ == "__main__":
    if codeql_sarif_contain_error(sys.argv[1]):
        sys.exit(1)
