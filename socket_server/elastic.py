from elasticsearch import Elasticsearch

es = Elasticsearch(
    "https://quickstart-es-http:9200",
    basic_auth=("elastic", "945cZ19Jloa9ikUD3JX9V7H9"),
    verify_certs=False,
    ssl_show_warn=False,
)

def parse_metrics(metrics_string, engine):
    parsed_metrics = {
        "engine": engine,
    }

    for metric in metrics_string.split(","):
        key, value = metric.split("=")
        parsed_metrics[key] = value

    return parsed_metrics

def send_metrics_to_elastic(metrics, engine):
    parsed_metrics = parse_metrics(metrics, engine)
    es.index(index="engines-perfomance-metrics", document=parsed_metrics)
