# Run this app with `python app.py` and
# visit http://127.0.0.1:8050/ in your web browser.

from dash import Dash, html, dcc, callback, Input, Output
import plotly.express as px
import pandas as pd
import dash_bootstrap_components as dbc
import sqlalchemy as sql

app = Dash(__name__)


def get_sql_engine():
    return sql.engine.create_engine(
        f"postgresql+psycopg2://postgres:letmein@timescaledb:5432/postgres"
    )

engine = get_sql_engine()

# assume you have a "long-form" data frame
# see https://plotly.com/python/px-arguments/ for more options

app.layout = dbc.Container(
    [
        html.H2(['Weather Tracker']),
        dcc.Dropdown(
            ['5S', '1 min', '5 min', '15 min'],
            '5S',
            id='granularity-dropdown'
        ),

        dcc.Dropdown(
            ['1 hour', '3 hours', '12 hours'],
            '1 hour',
            id='lookback-period-dropdown'
        ),
        dcc.Graph(id='temperature-graph'),
    
        

        # dcc.Interval(
        #     id='interval-component',
        #     interval=5*1000, # in milliseconds
        #     n_intervals=0
        # ),

        # html.Div(children=[],
        #     id='graphs-container'),
    ],
    fluid = True,
    className = "dbc",
)

@callback(
    Output("temperature-graph", "figure"),
    Input('granularity-dropdown', 'value'),
    Input('lookback-period-dropdown', 'value'),
)
def update_graph(granularity, lookback):
    temp_df = pd.read_sql(
        sql.text("SELECT time, sensor_id, metric_id, double FROM readings WHERE time >= now() - interval :lookback_period AND sensor_id=1 AND metric_id=1"), 
        engine.connect(),
        params={'lookback_period': lookback},
        index_col='time')
    return px.line(temp_df,
            x=temp_df.index, 
            y="double", 
            title='Temperature', 
            markers=True, 
            labels={'double': '° Celcius', 'time': 'Timestamp'},)

# @app.callback(
#     Output('graphs-container', 'children'),
#     Input('granularity-dropdown', 'value'),
#     Input('lookback-period-dropdown', 'value'),
#     Input('interval-component', 'n_intervals'),
# )
# def update_graphs(granularity, lookback_period, n):
#     temp_df = pd.read_sql(
#         sql.text("SELECT time, sensor_id, metric_id, double FROM readings WHERE time >= now() - interval :lookback_period AND sensor_id=1 AND metric_id=1"), 
#         engine.connect(),
#         params={'lookback_period': lookback_period},
#         index_col='time')
#     hum_df = pd.read_sql(
#         sql.text("SELECT time, sensor_id, metric_id, double FROM readings WHERE time >= now() - interval :lookback_period AND sensor_id=1 AND metric_id=2"), 
#         engine.connect(),
#         params={'lookback_period': lookback_period},
#         index_col='time')
#     pres_df = pd.read_sql(
#         sql.text("SELECT time, sensor_id, metric_id, double FROM readings WHERE time >= now() - interval :lookback_period AND sensor_id=1 AND metric_id=3"), 
#         engine.connect(),
#         params={'lookback_period': lookback_period},
#         index_col='time')
#     temp_re_df = temp_df.resample(granularity).mean()
#     hum_re_df = hum_df.resample(granularity).mean()
#     pres_re_df = pres_df.resample(granularity).mean()
#     temp_fig = px.line(
#         temp_re_df, 
#         x=temp_re_df.index, 
#         y="double", 
#         title='Temperature', 
#         markers=True, 
#         labels={'double': '° Celcius', 'time': 'Timestamp'},
#         template=theme,
#     )
#     hum_fig = px.line(
#         hum_re_df, 
#         x=hum_re_df.index, 
#         y="double", 
#         title='Humidity', 
#         markers=True, 
#         labels={'double': '% RH', 'time': 'Timestamp'},
#         template=theme,
#     )
#     pres_fig = px.line(
#         pres_re_df, 
#         x=pres_re_df.index, 
#         y="double", 
#         title='Barometric Pressure', 
#         markers=True, 
#         labels={'double': 'Pa', 'time': 'Timestamp'},
#         template=theme,
#     )
#     return [
#         dcc.Graph(
#             id='temperature-graph',
#             figure=temp_fig
#         ),
#         dcc.Graph(
#             id='humidity-graph',
#             figure=hum_fig
#         ),
#         dcc.Graph(
#             id='pressure-graph',
#             figure=pres_fig
#         ),
#     ]

if __name__ == '__main__':
    app.run_server(debug=True)